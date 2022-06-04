#include <psxgte.h>
#include <psxgpu.h>
#include <stdio.h>
#include <stdlib.h>
#include <inline_c.h>
#include "clip.h"
#include "psm.h"

#define OT_LEN			4094

int triCounter = 0;
u_char light = 40;

typedef union {
    unsigned char c[4];
    int i;
} raw_int;

typedef union {
    unsigned char c[2];
    short s;
} raw_short;

void LoadModel(unsigned long* data, MODEL **model) {
	HEADER* header = (HEADER*)data;
	printf("\nModel Info:\nUntextured tris: %d\nTextured tris: %d\nVerts: %d\nNorms: %d\nUV's: %d", header->numUntex, header->numTex, header->numVerts, header->numNorms, header->numUV);
    int fileSize = 11 + (header->numVerts * sizeof(VECTOR)) + (header->numNorms * sizeof(VECTOR)) + (header->numMat * sizeof(COLVECTOR)) + (header->numUV * sizeof(UV_COORDS)) + (header->numUntex * sizeof(FTRI)) + (header->numTex * sizeof(FTTRI));
	*model = malloc(fileSize);

    (*model)->h = malloc(11);
    (*model)->vIndex = malloc(header->numVerts * sizeof(VECTOR));
    (*model)->nIndex = malloc(header->numNorms * sizeof(VECTOR));
    (*model)->matIndex = malloc(header->numMat * sizeof(COLVECTOR));
    (*model)->uvIndex = malloc(header->numUV * sizeof(UV_COORDS));
    (*model)->untexFaces = malloc(header->numUntex * sizeof(FTRI));
    (*model)->texFaces = malloc(header->numTex * sizeof(FTTRI));

    (*model)->h = header;
    unsigned char* byteData = (unsigned char *)data;
    int index = 11;
    raw_int verts[3] = { 0 };
    raw_short triInfo[7] = { 0 };
    uint8_t counter = 0;

    for (int i = 0; i < (*model)->h->numVerts; i++) {
            for (int a = 0; a < 3; a++) {
                verts[a].c[0] = byteData[index];
                index++;
                verts[a].c[1] = byteData[index];
                index++;
                verts[a].c[2] = byteData[index];
                index++;
                verts[a].c[3] = byteData[index];
                index++;
            }

        (*model)->vIndex[i].vx = verts[0].i;
        (*model)->vIndex[i].vy = verts[1].i;
        (*model)->vIndex[i].vz = verts[2].i;
    }

    for (int i = 0; i < (*model)->h->numNorms; i++) {
        for (int a = 0; a < 3; a++) {
            verts[a].c[0] = byteData[index];
            index++;
            verts[a].c[1] = byteData[index];
            index++;
            verts[a].c[2] = byteData[index];
            index++;
            verts[a].c[3] = byteData[index];
            index++;
        }

        (*model)->nIndex[i].vx = verts[0].i;
        (*model)->nIndex[i].vy = verts[1].i;
        (*model)->nIndex[i].vz = verts[2].i;
    }

    for (int i = 0; i < (*model)->h->numMat; i++) {
        (*model)->matIndex[i].r = byteData[index];
        index++;
        (*model)->matIndex[i].b = byteData[index];
        index++;
        (*model)->matIndex[i].g = byteData[index];
        index++;
    }

    for (int i = 0; i < (*model)->h->numUV; i++) {
        (*model)->uvIndex[i].u = (uint8_t)byteData[index];
        index++;
        (*model)->uvIndex[i].v = (uint8_t)byteData[index];
        index++;
    }

    for (int i = 0; i < (*model)->h->numUntex; i++) {
        for (int a = 0; a < 4; a++) {
            triInfo[a].c[0] = byteData[index];
            index++;
            triInfo[a].c[1] = byteData[index];
            index++;
        }
        (*model)->untexFaces[i].v[0] = triInfo[0].s;
        (*model)->untexFaces[i].v[1] = triInfo[1].s;
        (*model)->untexFaces[i].v[2] = triInfo[2].s;
        (*model)->untexFaces[i].n = triInfo[3].s;
        (*model)->untexFaces[i].mat = byteData[index];
        index++;
    }

    for (int i = 0; i < (*model)->h->numTex; i++) {
        for (int a = 0; a < 7; a++) {
            triInfo[a].c[0] = byteData[index];
            index++;
            triInfo[a].c[1] = byteData[index];
            index++;
        }
        (*model)->texFaces[i].v[0] = triInfo[0].s;
        (*model)->texFaces[i].v[1] = triInfo[1].s;
        (*model)->texFaces[i].v[2] = triInfo[2].s;
        (*model)->texFaces[i].n = triInfo[3].s;
        (*model)->texFaces[i].t[0] = triInfo[4].s;
        (*model)->texFaces[i].t[1] = triInfo[5].s;
        (*model)->texFaces[i].t[2] = triInfo[6].s;
    }

    printf("\nInitialized");
    printf("\nModel Info:\nUntextured tris: %d\nTextured tris: %d\nVerts: %d\nNorms: %d\nUV's: %d", (*model)->h->numUntex, (*model)->h->numTex, (*model)->h->numVerts, (*model)->h->numNorms, (*model)->h->numUV);
    printf("\nSecond Face - x: %d  y: %d  z: %d  -  Norm: %d", (*model)->texFaces[1].v[0], (*model)->texFaces[1].v[1], (*model)->texFaces[1].v[2], (*model)->texFaces[1].n);
    printf("\nSecond Vert - x: %d  y: %d  z: %d", (*model)->vIndex[1].vx, (*model)->vIndex[1].vy, (*model)->vIndex[1].vz);
    printf("\n%d", (*model)->uvIndex[0].u);
}

void DrawModel_Unlit(MODEL* model, MATRIX* mtx, VECTOR* pos, SVECTOR* rot, RECT screen_clip, u_long* OT, char* db_nextpri, TIM_IMAGE tex) {
    int i, p;
    POLY_F3* pol3;
    POLY_FT3* polt3;

    // Object and light matrix for object
    MATRIX omtx, lmtx;

    // Set object rotation and position
    RotMatrix(rot, &omtx);
    TransMatrix(&omtx, pos);

    // Composite coordinate matrix transform, so object will be rotated and
    // positioned relative to camera matrix (mtx), so it'll appear as
    // world-space relative.
    CompMatrixLV(mtx, &omtx, &omtx);

    // Save matrix
    PushMatrix();

    // Set matrices
    gte_SetRotMatrix(&omtx);
    gte_SetTransMatrix(&omtx);

    // Sort the cube
    pol3 = (POLY_F3*)db_nextpri;
    
    if (model->h->numUntex != 0) {
        printf("\nUntex");
        for (i = 0; i < model->h->numUntex; i++) {

            // Load the first 3 vertices of a quad to the GTE 
            gte_ldv3(
                &model->vIndex[model->untexFaces[i].v[0] - 1],
                &model->vIndex[model->untexFaces[i].v[1] - 1],
                &model->vIndex[model->untexFaces[i].v[2] - 1]);

            // Rotation, Translation and Perspective Triple
            gte_rtpt();

            // Compute normal clip for backface culling
            gte_nclip();

            // Get result
            gte_stopz(&p);

            // Skip this face if backfaced
            if (p < 0)
                continue;

            // Calculate average Z for depth sorting
            gte_avsz3();
            gte_stotz(&p);

            // Skip if clipping off
            // (the shift right operator is to scale the depth precision)
            if (((p >> 2) <= 0) || ((p >> 2) >= OT_LEN))
                continue;

            // Initialize a tri primitive
            setPolyF3(pol3);

            // Set the projected vertices to the primitive
            gte_stsxy0(&pol3->x0);
            gte_stsxy1(&pol3->x1);
            gte_stsxy2(&pol3->x2);

            // Test if quad is off-screen, discard if so
            if (tri_clip(&screen_clip,
                (DVECTOR*)&pol3->x0, (DVECTOR*)&pol3->x1,
                (DVECTOR*)&pol3->x2))
                continue;

            // Load primitive color even though gte_ncs() doesn't use it.
            // This is so the GTE will output a color result with the
            // correct primitive code.
            gte_ldrgb(&pol3->r0);

            SVECTOR norm = (SVECTOR){ model->nIndex[model->untexFaces[i].n - 1].vx, model->nIndex[model->untexFaces[i].n - 1].vy, model->nIndex[model->untexFaces[i].n - 1].vz, 0 };
            // Load the face normal
            gte_ldv0(&norm);

            gte_avsz4();
            gte_stotz(&p);

            setRGB0(pol3, model->matIndex[model->untexFaces[i].mat].r, model->matIndex[model->untexFaces[i].mat].g, model->matIndex[model->untexFaces[i].mat].b);

            // Sort primitive to the ordering table
            addPrim(OT + (p >> 2), pol3);

            // Advance to make another primitive
            pol3++;

        }
    }

    // Update nextpri
    db_nextpri = (char*)pol3;

    polt3 = (POLY_FT3*)db_nextpri;

    

    for (i = 0; i < model->h->numTex; i++) {

        // Load the first 3 vertices of a quad to the GTE
        if (light < 254) {
            SVECTOR v1 = (SVECTOR){ model->vIndex[model->texFaces[i].v[0] - 1].vx,model->vIndex[model->texFaces[i].v[0] - 1].vy,model->vIndex[model->texFaces[i].v[0] - 1].vz,0 };
            SVECTOR v2 = (SVECTOR){ model->vIndex[model->texFaces[i].v[1] - 1].vx,model->vIndex[model->texFaces[i].v[1] - 1].vy,model->vIndex[model->texFaces[i].v[1] - 1].vz,0 };
            SVECTOR v3 = (SVECTOR){ model->vIndex[model->texFaces[i].v[2] - 1].vx,model->vIndex[model->texFaces[i].v[2] - 1].vy,model->vIndex[model->texFaces[i].v[2] - 1].vz,0 };
            gte_ldv3(
                &v1,
                &v2,
                &v3);

            // Rotation, Translation and Perspective Triple
            gte_rtpt();

            // Compute normal clip for backface culling
            gte_nclip();

            // Get result
            gte_stopz(&p);

            // Skip this face if backfaced
            if (p <= 0)
                continue;

            

            // Calculate average Z for depth sorting
            gte_avsz3();
            gte_stotz(&p);

            // Skip if clipping off
            // (the shift right operator is to scale the depth precision)
            if (((p >> 2) <= 15) || ((p >> 2) >= OT_LEN))
                continue;

            // Initialize a tri primitive


            // Set the projected vertices to the primitive
            gte_stsxy3(&polt3->x0, &polt3->x1, &polt3->x2);
            /*gte_stsxy0(&polt3->x0);
            gte_stsxy1(&polt3->x1);
            gte_stsxy2(&polt3->x2);*/

            // Test if tri is off-screen, discard if so
            if (tri_clip(&screen_clip,
                (DVECTOR*)&polt3->x0, (DVECTOR*)&polt3->x1,
                (DVECTOR*)&polt3->x2))
                continue;

            // Load primitive color even though gte_ncs() doesn't use it.
            // This is so the GTE will output a color result with the
            // correct primitive code.
            gte_ldrgb(&polt3->r0);

            /*SVECTOR norm = (SVECTOR){model->nIndex[model->texFaces[i].n - 1].vx, model->nIndex[model->texFaces[i].n - 1].vy, model->nIndex[model->texFaces[i].n - 1].vz, 0};

            // Load the face normal
            gte_ldv0(&norm);*/


            setUV3(polt3, model->uvIndex[model->texFaces[i].t[0] - 1].u, model->uvIndex[model->texFaces[i].t[0] - 1].v, model->uvIndex[model->texFaces[i].t[1] - 1].u, model->uvIndex[model->texFaces[i].t[1] - 1].v, model->uvIndex[model->texFaces[i].t[2] - 1].u, model->uvIndex[model->texFaces[i].t[2] - 1].v);

            setClut(polt3, tex.crect->x, tex.crect->y);

            setTPage(polt3, tex.mode & 0x3, 1, tex.prect->x, tex.prect->y);
            setRGB0(polt3, -light, -light, -light);
            setPolyFT3(polt3);


            // Sort primitive to the ordering table
            addPrim(OT + (p >> 2), polt3);

            // Advance to make another primitive
            polt3++;

            triCounter++;
            if (light < 254) {
                light = triCounter + 40;
            }
            //printf("\nTri %d:  x1: %d y1: %d z1: %d  x2: %d y2: %d z2: %d  x3: %d y3: %d z3: %d", i, model->vIndex[model->texFaces[i].v[0] - 1].vx, model->vIndex[model->texFaces[i].v[0] - 1].vy, model->vIndex[model->texFaces[i].v[0] - 1].vz, model->vIndex[model->texFaces[i].v[1] - 1].vx, model->vIndex[model->texFaces[i].v[1] - 1].vy, model->vIndex[model->texFaces[i].v[1] - 1].vz, model->vIndex[model->texFaces[i].v[2] - 1].vx, model->vIndex[model->texFaces[i].v[2] - 1].vy, model->vIndex[model->texFaces[i].v[2] - 1].vz);
        }
    }

    // Update nextpri
    db_nextpri = (char*)polt3;
    printf(triCounter);
    triCounter = 0;
    light = 40;

    // Restore matrix
    PopMatrix();
}
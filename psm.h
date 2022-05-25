#pragma once
#include <stdint.h>

typedef struct FTRI {
	short v[3];
	short n;
	uint8_t mat;
} FTRI; //Struct for untextured polys

typedef struct FTTRI {
	short v[3];
	short n;
	short t[3];
} FTTRI; //Struct for textured polys

typedef struct UV_COORDS {
	uint8_t u, v;
} UV_COORDS; //Coords for Textured Polys

typedef struct HEADER {
	short numUntex;
	short numTex;
	short numVerts;
	short numNorms;
	short numUV;
	uint8_t numMat;
} HEADER; //File Header

typedef struct COLVECTOR {
	unsigned char r, g, b;
} COLVECTOR; //Simple Colour Vector

typedef struct MODEL {
	HEADER* h;
	VECTOR* vIndex;
	VECTOR* nIndex;
	COLVECTOR* matIndex;
	UV_COORDS* uvIndex;
	FTRI* untexFaces;
	FTTRI* texFaces;
} MODEL; //PSM file struct

void LoadModel(unsigned long* data, MODEL** model);
void DrawModel_Unlit(MODEL* model, MATRIX* mtx, VECTOR* pos, SVECTOR* rot, RECT screen_clip, u_long* OT, char* db_nextpri, u_long tpage, u_long clutid);
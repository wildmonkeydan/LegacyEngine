# PSn00bSDK example CMake script
# (C) 2021 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.20)

project(
	SataliteScramble
	LANGUAGES    C CXX ASM
	VERSION      1.0.0
	DESCRIPTION  "Twin Stick Shooter"
)

file(GLOB _sources *.c)
psn00bsdk_add_executable(template STATIC ${_sources})

psn00bsdk_add_cd_image(
	iso      # Target name
	template # Output file name (= template.bin + template.cue)
	iso.xml  # Path to config file
	DEPENDS template
)

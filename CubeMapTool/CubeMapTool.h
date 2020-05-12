#pragma once
#include <windows.h>


void GenerateIrradianceEnvMapSH(const gli::texture2d &texture, float *sh_red, float *sh_grn, float *sh_blu, int samples);
void GenerateIrradianceCubeMapSH(const gli::texture_cube &texture, float *sh_red, float *sh_grn, float *sh_blu, int samples);

BOOL GenerateEnvIrradianceMap(gli::texture2d &texEnvMap, gli::texture_cube &texIrrMap, int samples);
BOOL GenerateCubeIrradianceMap(gli::texture_cube &texCubeMap, gli::texture_cube &texIrrMap, int samples);
BOOL GenerateEnvMipmaps(gli::texture2d &texEnvMap, gli::texture2d &texEnvMipmap, int samples);
BOOL GenerateCubeMipmaps(gli::texture_cube &texCubeMap, gli::texture_cube &texCubeMipmap, int samples);
BOOL GenerateLutMap(gli::texture2d &texLutMap, int samples);

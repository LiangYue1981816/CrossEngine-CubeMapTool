#pragma once
#include <windows.h>


BOOL GenerateEnvIrradianceMap(gli::texture2d &texEnvMap, gli::texture_cube &texIrrMap, int samples);
BOOL GenerateCubeIrradianceMap(gli::texture_cube &texCubeMap, gli::texture_cube &texIrrMap, int samples);
BOOL GenerateEnvMipmaps(gli::texture2d &texEnv, gli::texture2d &texEnvMipmap, int samples);
BOOL GenerateCubeMipmaps(CUBEMAP *pCubeMap, CUBEMAP pMipmaps[], int mipLevels, int samples);
BOOL GenerateLutMap(gli::texture2d &texLutMap, int samples);

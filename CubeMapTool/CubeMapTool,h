#pragma once
#include <windows.h>


BOOL GenerateEnvIrradianceMap(gli::texture2d &texEnvMap, gli::texture_cube &texIrrMap, int samples);
BOOL GenerateCubeIrradianceMap(gli::texture_cube &texCubeMap, gli::texture_cube &texIrrMap, int samples);
BOOL GenerateEnvMipmaps(IMAGE *pEnvMap, IMAGE pMipmaps[], int mipLevels, int samples);
BOOL GenerateCubeMipmaps(CUBEMAP *pCubeMap, CUBEMAP pMipmaps[], int mipLevels, int samples);
BOOL GenerateLUT(IMAGE *pImage, int samples);

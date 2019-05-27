#pragma once

#include "targetver.h"

#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <windows.h>

#include "glew.h"
#include "glut.h"
#include "Texture.h"


#define PI 3.1415926535897932384626433832795f


#include "IMAGE.H"
#define CUBEMAP_WIDTH(cubemap) IMAGE_WIDTH(&(cubemap)->faces[0])
#define CUBEMAP_HEIGHT(cubemap) IMAGE_HEIGHT(&(cubemap)->faces[0])
#define CUBEMAP_BITCOUNT(cubemap) IMAGE_BITCOUNT(&(cubemap)->faces[0])
struct CUBEMAP {
	IMAGE faces[6];
};
void CubeMapInit(CUBEMAP *pCubeMap);
void CubeMapFree(CUBEMAP *pCubeMap);
BOOL CubeMapAlloc(CUBEMAP *pCubeMap, int width, int height, int bitcount);
BOOL CubeMapLoad(CUBEMAP *pCubeMap, char szFileNames[6][_MAX_PATH]);
unsigned long CubeMapGetPixelColor(CUBEMAP *pCubeMap, glm::vec3 &texcoord);
void PreviewMap(CUBEMAP *pCubeMap, IMAGE *imgPreview);
GLuint CreateTexture2D(IMAGE *pImage);
GLuint CreateTextureCube(CUBEMAP *pCubeMap);
void DestroyTexture(GLuint texture);



GLuint GLCreateTexture2D(const gli::texture2d &texture);
GLuint GLCreateTextureCube(const gli::texture_cube &texture);
void GLDestroyTexture(GLuint texture);

extern GLuint rbo;
extern GLuint fbo;
extern GLuint fboTexture;
extern GLuint fboTextureWidth;
extern GLuint fboTextureHeight;
BOOL GLCreateFBO(int width, int height, gli::format format);
void GLDestroyFBO(void);

extern GLuint program;
extern GLuint attribLocationPosition;
extern GLuint attribLocationTexcoord;
extern GLuint uniformLocationTexture;
extern GLuint uniformLocationTexcoordMatrix;
extern GLuint uniformLocationModelViewProjectionMatrix;
extern GLuint uniformLocationSHRed;
extern GLuint uniformLocationSHGrn;
extern GLuint uniformLocationSHBlu;
extern GLuint uniformLocationSamples;
extern GLuint uniformLocationRoughness;
extern GLuint uniformLocationEnvmap;
extern GLuint uniformLocationCubemap;
BOOL GLCreateProgram(const char *szShaderVertexCode, const char *szShaderFragmentCode);
void GLDestroyProgram(void);

extern GLuint ibo;
extern GLuint vbo;
typedef struct vertex { float position[3]; float texcoord[2]; } vertex;
BOOL GLCreateVBO(const vertex *vertices, int numVertices, const unsigned short *indices, int numIndices);
void GLDestroyVBO(void);

BOOL GenerateEnvIrradianceMap(gli::texture2d &texEnvMap, gli::texture_cube &texIrrMap, int samples);
BOOL GenerateCubeIrradianceMap(gli::texture_cube &texCubeMap, gli::texture_cube &texIrrMap, int samples);
BOOL GenerateEnvMipmaps(IMAGE *pEnvMap, IMAGE pMipmaps[], int mipLevels, int samples);
BOOL GenerateCubeMipmaps(CUBEMAP *pCubeMap, CUBEMAP pMipmaps[], int mipLevels, int samples);
BOOL GenerateLUT(IMAGE *pImage, int samples);

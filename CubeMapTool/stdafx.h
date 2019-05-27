#pragma once

#include "targetver.h"

#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <windows.h>

#include "glew.h"
#include "glut.h"
#include "gli/gli.hpp"
#include "gli/convert.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"


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


#define PI 3.1415926535897932384626433832795f


#define TEXTURE_CUBE_MAP_POSITIVE_X 0
#define TEXTURE_CUBE_MAP_NEGATIVE_X 1
#define TEXTURE_CUBE_MAP_POSITIVE_Y 2
#define TEXTURE_CUBE_MAP_NEGATIVE_Y 3
#define TEXTURE_CUBE_MAP_POSITIVE_Z 4
#define TEXTURE_CUBE_MAP_NEGATIVE_Z 5


template <typename texture_type> texture_type LoadTexture(const char *szFileName);
template <typename texture_type> void SaveTextureDDS(const char *szFileName, const texture_type &texture);
template <typename texture_type> void SaveTextureKTX(const char *szFileName, const texture_type &texture);
template <typename texture_type> void SaveTextureKMG(const char *szFileName, const texture_type &texture);
void SetTexturePixelColor(gli::texture2d &texture, int x, int y, int level, const glm::f32vec3 &color);
void SetTexturePixelColor(gli::texture_cube &texture, int x, int y, int face, int level, const glm::f32vec3 &color);
glm::f32vec3 GetTexturePixelColor(const gli::texture2d &texture, int x, int y);
glm::f32vec3 GetTexturePixelColor(const gli::texture_cube &texture, int x, int y, int face);
glm::f32vec3 GetTexturePixelColor(const gli::texture_cube &texture, const glm::vec3 &direction);
gli::texture2d ConvertTextureCubeToTexture2D(const gli::texture_cube &cube);

GLuint GLCreateTexture2D(const gli::texture2d &texture);
GLuint GLCreateTextureCube(const gli::texture_cube &texture);
void GLDestroyTexture(GLuint texture);

extern GLuint rbo;
extern GLuint fbo;
extern GLuint fboTexture;
extern GLuint fboTextureWidth;
extern GLuint fboTextureHeight;
BOOL GLCreateFBO(int width, int height);
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

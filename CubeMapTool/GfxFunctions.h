#pragma once
#include "glew.h"
#include "glut.h"
#include <windows.h>


BOOL GLCreateTexture2D(const gli::texture2d &texture, GLuint &tex);
BOOL GLCreateTextureCube(const gli::texture_cube &texture, GLuint &tex);
void GLDestroyTexture(GLuint tex);

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

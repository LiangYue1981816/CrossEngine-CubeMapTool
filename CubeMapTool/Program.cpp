#include "stdafx.h"


GLuint program = 0;
GLuint attribLocationPosition = 0;
GLuint attribLocationTexcoord = 0;
GLuint uniformLocationTexture = 0;
GLuint uniformLocationTexcoordMatrix = 0;
GLuint uniformLocationModelViewProjectionMatrix = 0;
GLuint uniformLocationSHRed = 0;
GLuint uniformLocationSHGrn = 0;
GLuint uniformLocationSHBlu = 0;
GLuint uniformLocationSamples = 0;
GLuint uniformLocationRoughness = 0;
GLuint uniformLocationEnvmap = 0;

BOOL CreateProgram(const char *szShaderVertexCode, const char *szShaderFragmentCode)
{
	GLint linked;
	GLint compiled;
	GLuint vertex;
	GLuint fragment;

	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &szShaderVertexCode, NULL);
	glCompileShader(vertex);
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &compiled);
	if (compiled == GL_FALSE) {
		GLint len;
		GLchar szError[4 * 1024];
		glGetShaderiv(vertex, GL_INFO_LOG_LENGTH, &len);
		glGetShaderInfoLog(vertex, sizeof(szError), &len, szError);
		glDeleteShader(vertex);
		printf("Vertex Error: %s\n", szError);
		vertex = 0;
		return FALSE;
	}

	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &szShaderFragmentCode, NULL);
	glCompileShader(fragment);
	glGetShaderiv(fragment, GL_COMPILE_STATUS, &compiled);
	if (compiled == GL_FALSE) {
		GLint len;
		GLchar szError[4 * 1024];
		glGetShaderiv(fragment, GL_INFO_LOG_LENGTH, &len);
		glGetShaderInfoLog(fragment, sizeof(szError), &len, szError);
		glDeleteShader(fragment);
		printf("Fragment Error: %s\n", szError);
		fragment = 0;
		return FALSE;
	}

	program = glCreateProgram();
	glAttachShader(program, vertex);
	glAttachShader(program, fragment);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (linked == GL_FALSE) return FALSE;

	attribLocationPosition = glGetAttribLocation(program, "_position");
	attribLocationTexcoord = glGetAttribLocation(program, "_texcoord");
	uniformLocationTexture = glGetUniformLocation(program, "_texture");
	uniformLocationTexcoordMatrix = glGetUniformLocation(program, "_texcoordMatrix");
	uniformLocationModelViewProjectionMatrix = glGetUniformLocation(program, "_modelViewProjectionMatrix");
	uniformLocationSHRed = glGetUniformLocation(program, "_sh_red");
	uniformLocationSHGrn = glGetUniformLocation(program, "_sh_grn");
	uniformLocationSHBlu = glGetUniformLocation(program, "_sh_blu");
	uniformLocationSamples = glGetUniformLocation(program, "_samples");
	uniformLocationRoughness = glGetUniformLocation(program, "_roughness");
	uniformLocationEnvmap = glGetUniformLocation(program, "_envmap");

	return TRUE;
}

void DestroyProgram(void)
{
	glUseProgram(0);
	glDeleteProgram(program);

	program = 0;
	attribLocationPosition = 0;
	attribLocationTexcoord = 0;
	uniformLocationTexture = 0;
	uniformLocationTexcoordMatrix = 0;
	uniformLocationModelViewProjectionMatrix = 0;
	uniformLocationSHRed = 0;
	uniformLocationSHGrn = 0;
	uniformLocationSHBlu = 0;
}

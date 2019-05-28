#include "stdafx.h"


BOOL GLCreateTexture2D(const gli::texture2d &texture, GLuint &tex)
{
	gli::gl GL(gli::gl::PROFILE_ES30);
	gli::gl::format format = GL.translate(texture.format(), texture.swizzles());

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, format.Internal, texture.extent().x, texture.extent().y, 0, format.External, format.Type, texture.data());
	glBindTexture(GL_TEXTURE_2D, 0);

	return TRUE;
}

BOOL GLCreateTextureCube(const gli::texture_cube &texture, GLuint &tex)
{
	gli::gl GL(gli::gl::PROFILE_ES30);
	gli::gl::format format = GL.translate(texture.format(), texture.swizzles());

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, format.Internal, texture.extent().x, texture.extent().y, 0, format.External, format.Type, texture.data(0, 0, 0));
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, format.Internal, texture.extent().x, texture.extent().y, 0, format.External, format.Type, texture.data(0, 1, 0));
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, format.Internal, texture.extent().x, texture.extent().y, 0, format.External, format.Type, texture.data(0, 2, 0));
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, format.Internal, texture.extent().x, texture.extent().y, 0, format.External, format.Type, texture.data(0, 3, 0));
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, format.Internal, texture.extent().x, texture.extent().y, 0, format.External, format.Type, texture.data(0, 4, 0));
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, format.Internal, texture.extent().x, texture.extent().y, 0, format.External, format.Type, texture.data(0, 5, 0));
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return TRUE;
}

void GLDestroyTexture(GLuint tex)
{
	glDeleteTextures(1, &tex);
}


GLuint rbo = 0;
GLuint fbo = 0;
GLuint fboTexture = 0;
GLuint fboTextureWidth = 0;
GLuint fboTextureHeight = 0;

BOOL GLCreateFBO(int width, int height, gli::format format)
{
	gli::gl GL(gli::gl::PROFILE_ES30);
	gli::gl::format glFormat = GL.translate(format);

	fboTextureWidth = width;
	fboTextureHeight = height;

	glGenTextures(1, &fboTexture);
	glBindTexture(GL_TEXTURE_2D, fboTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, glFormat.Internal, fboTextureWidth, fboTextureHeight, 0, glFormat.External, glFormat.Type, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, fboTextureWidth, fboTextureHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE ? TRUE : FALSE;
}

void GLDestroyFBO(void)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &fbo);
	glDeleteRenderbuffers(1, &rbo);

	rbo = 0;
	fbo = 0;
	fboTexture = 0;
	fboTextureWidth = 0;
	fboTextureHeight = 0;
}


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
GLuint uniformLocationCubemap = 0;

BOOL GLCreateProgram(const char *szShaderVertexCode, const char *szShaderFragmentCode)
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
	uniformLocationCubemap = glGetUniformLocation(program, "_cubemap");

	return TRUE;
}

void GLDestroyProgram(void)
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


GLuint ibo = 0;
GLuint vbo = 0;

BOOL GLCreateVBO(const vertex *vertices, int numVertices, const unsigned short *indices, int numIndices)
{
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * numVertices, vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * numIndices, indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return TRUE;
}

void GLDestroyVBO(void)
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &ibo);
	glDeleteBuffers(1, &vbo);

	ibo = 0;
	vbo = 0;
}

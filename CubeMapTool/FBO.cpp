#include "stdafx.h"


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

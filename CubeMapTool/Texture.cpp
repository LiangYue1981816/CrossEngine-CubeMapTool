#include "stdafx.h"


void FlipImage(IMAGE *pImage)
{
	for (int y = 0; y < IMAGE_HEIGHT(pImage) / 2; y++) {
		unsigned char *pPixelTop = pImage->data + pImage->buffWidth * y;
		unsigned char *pPixelBottom = pImage->data + pImage->buffWidth * (IMAGE_HEIGHT(pImage) - y - 1);

		for (int x = 0; x < pImage->buffWidth; x++) {
			unsigned char data = pPixelTop[x];
			pPixelTop[x] = pPixelBottom[x];
			pPixelBottom[x] = data;
		}
	}
}

GLuint CreateTexture2D(IMAGE *pImage)
{
	GLuint texture;

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	{
		FlipImage(pImage);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, IMAGE_WIDTH(pImage), IMAGE_HEIGHT(pImage), 0, GL_BGR, GL_UNSIGNED_BYTE, pImage->data);
		FlipImage(pImage);
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	return texture;
}

GLuint CreateTextureCube(CUBEMAP *pCubeMap)
{
	GLuint texture;

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
	{
		for (int index = 0; index < 6; index++) {
			FlipImage(&pCubeMap->faces[index]);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + index, 0, GL_RGB, CUBEMAP_WIDTH(pCubeMap), CUBEMAP_HEIGHT(pCubeMap), 0, GL_BGR, GL_UNSIGNED_BYTE, pCubeMap->faces[index].data);
			FlipImage(&pCubeMap->faces[index]);
		}
	}
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return texture;
}

void DestroyTexture(GLuint texture)
{
	glDeleteTextures(1, &texture);
}

#include "stdafx.h"


template <typename texture_type>
texture_type LoadTexture(const char *szFileName)
{
	texture_type texture = (texture_type)gli::load(szFileName);
	if (texture.empty()) return texture_type();
	if (gli::is_compressed(texture.format())) return texture_type();
	return gli::convert<texture_type>(texture, gli::FORMAT_RGBA32_SFLOAT_PACK32);
}

glm::f32vec4 GetTexturePixelColor(const gli::texture2d &texture, int x, int y)
{
	return texture.load<glm::f32vec4>(gli::texture2d::extent_type(x, y), 0);
}

glm::f32vec4 GetTexturePixelColor(const gli::texture_cube &texture, int x, int y, int face)
{
	return texture.load<glm::f32vec4>(gli::texture_cube::extent_type(x, y), face, 0);
}

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

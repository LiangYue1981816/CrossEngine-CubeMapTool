#include "stdafx.h"


void SetTexturePixelColor(gli::texture2d &texture, int x, int y, int level, const glm::f32vec3 &color)
{
	texture.store(gli::extent2d(x, y), level, color);
}

void SetTexturePixelColor(gli::texture_cube &texture, int x, int y, int face, int level, const glm::f32vec3 &color)
{
	texture.store(gli::extent2d(x, y), face, level, color);
}

glm::f32vec3 GetTexturePixelColor(const gli::texture2d &texture, int x, int y)
{
	return texture.load<glm::f32vec3>(gli::texture2d::extent_type(x, y), 0);
}

glm::f32vec3 GetTexturePixelColor(const gli::texture_cube &texture, int x, int y, int face)
{
	return texture.load<glm::f32vec3>(gli::texture_cube::extent_type(x, y), face, 0);
}

glm::f32vec3 GetTexturePixelColor(const gli::texture_cube &texture, const glm::vec3 &direction)
{
	/*
	https://en.wikipedia.org/wiki/Cube_mapping

	major axis
	direction     target                             sc     tc    ma
	----------    -------------------------------    ---    ---   ---
	+rx           TEXTURE_CUBE_MAP_POSITIVE_X_EXT    -rz    -ry   rx
	-rx           TEXTURE_CUBE_MAP_NEGATIVE_X_EXT    +rz    -ry   rx
	+ry           TEXTURE_CUBE_MAP_POSITIVE_Y_EXT    +rx    +rz   ry
	-ry           TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT    +rx    -rz   ry
	+rz           TEXTURE_CUBE_MAP_POSITIVE_Z_EXT    +rx    -ry   rz
	-rz           TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT    -rx    -ry   rz
	*/

	const float rx = direction.x;
	const float ry = direction.y;
	const float rz = direction.z;
	const float arx = fabs(rx);
	const float ary = fabs(ry);
	const float arz = fabs(rz);

	int face;
	int x, y;
	float sc, tc, ma;

	if (arx >= ary && arx >= arz) {
		if (rx >= 0.0f) {
			face = TEXTURE_CUBE_MAP_POSITIVE_X;
			sc = -rz;
			tc = -ry;
			ma = arx;
		}
		else {
			face = TEXTURE_CUBE_MAP_NEGATIVE_X;
			sc = rz;
			tc = -ry;
			ma = arx;
		}
	}
	else if (ary >= arx && ary >= arz) {
		if (ry >= 0.0f) {
			face = TEXTURE_CUBE_MAP_POSITIVE_Y;
			sc = rx;
			tc = rz;
			ma = ary;
		}
		else {
			face = TEXTURE_CUBE_MAP_NEGATIVE_Y;
			sc = rx;
			tc = -rz;
			ma = ary;
		}
	}
	else {
		if (rz >= 0.0f) {
			face = TEXTURE_CUBE_MAP_POSITIVE_Z;
			sc = rx;
			tc = -ry;
			ma = arz;
		}
		else {
			face = TEXTURE_CUBE_MAP_NEGATIVE_Z;
			sc = -rx;
			tc = -ry;
			ma = arz;
		}
	}

	x = (int)(((sc / ma + 1.0f) * 0.5f) * texture.extent().x);
	y = (int)(((tc / ma + 1.0f) * 0.5f) * texture.extent().y);

	x = max(x, 0);
	x = min(x, texture.extent().x - 1);

	y = max(y, 0);
	y = min(y, texture.extent().y - 1);

	return texture.load<glm::f32vec3>(gli::texture_cube::extent_type(x, y), face, 0);
}

gli::texture2d ConvertTextureCubeToTexture2D(const gli::texture_cube &cube)
{
	//     +Y
	// -X  +Z  +X  -Z
	//     -Y

	gli::texture2d texture(cube.format(), gli::extent2d(cube.extent().x * 4, cube.extent().y * 3));
	texture.copy(cube, 0, TEXTURE_CUBE_MAP_POSITIVE_X, 0, gli::extent3d(0, 0, 0), 0, 0, 0, gli::extent3d(cube.extent().x * 2, cube.extent().y * 1, 0), gli::extent3d(cube.extent().x, cube.extent().y, 0));
	texture.copy(cube, 0, TEXTURE_CUBE_MAP_NEGATIVE_X, 0, gli::extent3d(0, 0, 0), 0, 0, 0, gli::extent3d(cube.extent().x * 0, cube.extent().y * 1, 0), gli::extent3d(cube.extent().x, cube.extent().y, 0));
	texture.copy(cube, 0, TEXTURE_CUBE_MAP_POSITIVE_Y, 0, gli::extent3d(0, 0, 0), 0, 0, 0, gli::extent3d(cube.extent().x * 1, cube.extent().y * 0, 0), gli::extent3d(cube.extent().x, cube.extent().y, 0));
	texture.copy(cube, 0, TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, gli::extent3d(0, 0, 0), 0, 0, 0, gli::extent3d(cube.extent().x * 1, cube.extent().y * 2, 0), gli::extent3d(cube.extent().x, cube.extent().y, 0));
	texture.copy(cube, 0, TEXTURE_CUBE_MAP_POSITIVE_Z, 0, gli::extent3d(0, 0, 0), 0, 0, 0, gli::extent3d(cube.extent().x * 1, cube.extent().y * 1, 0), gli::extent3d(cube.extent().x, cube.extent().y, 0));
	texture.copy(cube, 0, TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, gli::extent3d(0, 0, 0), 0, 0, 0, gli::extent3d(cube.extent().x * 3, cube.extent().y * 1, 0), gli::extent3d(cube.extent().x, cube.extent().y, 0));
	return texture;
}

GLuint GLCreateTexture2D(const gli::texture2d &texture)
{
	gli::gl GL(gli::gl::PROFILE_ES30);
	gli::gl::format format = GL.translate(texture.format(), texture.swizzles());

	GLuint tex = 0;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, format.Internal, texture.extent().x, texture.extent().y, 0, format.External, format.Type, texture.data());
	glBindTexture(GL_TEXTURE_2D, 0);
	return tex;
}

GLuint GLCreateTextureCube(const gli::texture_cube &texture)
{
	gli::gl GL(gli::gl::PROFILE_ES30);
	gli::gl::format format = GL.translate(texture.format(), texture.swizzles());

	GLuint tex = 0;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, format.Internal, texture.extent().x, texture.extent().y, 0, format.External, format.Type, texture.data(0, 0, 0));
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, format.Internal, texture.extent().x, texture.extent().y, 0, format.External, format.Type, texture.data(0, 1, 0));
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, format.Internal, texture.extent().x, texture.extent().y, 0, format.External, format.Type, texture.data(0, 2, 0));
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, format.Internal, texture.extent().x, texture.extent().y, 0, format.External, format.Type, texture.data(0, 3, 0));
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, format.Internal, texture.extent().x, texture.extent().y, 0, format.External, format.Type, texture.data(0, 4, 0));
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, format.Internal, texture.extent().x, texture.extent().y, 0, format.External, format.Type, texture.data(0, 5, 0));
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	return tex;
}

void GLDestroyTexture(GLuint texture)
{
	glDeleteTextures(1, &texture);
}


// ====


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

#include "stdafx.h"


gli::texture2d LoadTexture2D(const char *szFileName)
{
	gli::texture2d texture = (gli::texture2d)gli::load(szFileName);
	if (texture.empty()) return gli::texture2d();
	if (gli::is_compressed(texture.format())) return gli::texture2d();

	if (gli::is_float(texture.format())) {
		texture = gli::convert<gli::texture2d>(texture, gli::FORMAT_RGB32_SFLOAT_PACK32);
//		texture = ConvertLinearToGamma(texture);
	}
	else {
		texture = gli::convert<gli::texture2d>(texture, gli::FORMAT_RGB32_SFLOAT_PACK32);
	}

	return texture;
}

gli::texture_cube LoadTextureCube(const char *szFileName)
{
	gli::texture_cube texture = (gli::texture_cube)gli::load(szFileName);
	if (texture.empty()) return gli::texture_cube();
	if (gli::is_compressed(texture.format())) return gli::texture_cube();

	if (gli::is_float(texture.format())) {
		texture = gli::convert<gli::texture_cube>(texture, gli::FORMAT_RGB32_SFLOAT_PACK32);
//		texture = ConvertLinearToGamma(texture);
	}
	else {
		texture = gli::convert<gli::texture_cube>(texture, gli::FORMAT_RGB32_SFLOAT_PACK32);
	}

	return texture;
}

gli::texture2d ConvertLinearToGamma(const gli::texture2d &texture)
{
	gli::texture2d texConvert(texture.format(), texture.extent());
	{
		for (int y = 0; y < texture.extent().y; y++) {
			for (int x = 0; x < texture.extent().x; x++) {
				gli::f32vec3 color = texture.load<glm::f32vec3>(gli::texture2d::extent_type(x, y), 0);
				color = gli::convertLinearToSRGB(color);
				texConvert.store(gli::extent2d(x, y), 0, color);
			}
		}
	}
	return texConvert;
}

gli::texture_cube ConvertLinearToGamma(const gli::texture_cube &texture)
{
	gli::texture_cube texConvert(texture.format(), texture.extent());
	{
		for (int face = 0; face < 6; face++) {
			for (int y = 0; y < texture.extent().y; y++) {
				for (int x = 0; x < texture.extent().x; x++) {
					gli::f32vec3 color = texture.load<glm::f32vec3>(gli::texture_cube::extent_type(x, y), face, 0);
					color = gli::convertLinearToSRGB(color);
					texConvert.store(gli::extent2d(x, y), face, 0, color);
				}
			}
		}
	}
	return texConvert;
}

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

	x = std::max(x, 0);
	x = std::min(x, texture.extent().x - 1);

	y = std::max(y, 0);
	y = std::min(y, texture.extent().y - 1);

	return texture.load<glm::f32vec3>(gli::texture_cube::extent_type(x, y), face, 0);
}

gli::texture2d Preview(const gli::texture_cube &cube)
{
	//     +Y
	// -X  +Z  +X  -Z
	//     -Y

	gli::texture2d texture(cube.format(), gli::extent2d(cube.extent().x * 4, cube.extent().y * 3));

	for (int y = 0; y < cube.extent().y; y++) {
		for (int x = 0; x < cube.extent().x; x++) {
			// TEXTURE_CUBE_MAP_POSITIVE_X
			{
				int xx = x + cube.extent().x * 2;
				int yy = y + cube.extent().y * 1;
				gli::f32vec3 color = GetTexturePixelColor(cube, x, y, TEXTURE_CUBE_MAP_POSITIVE_X);
				SetTexturePixelColor(texture, xx, yy, 0, color);
			}
			// TEXTURE_CUBE_MAP_NEGATIVE_X
			{
				int xx = x + cube.extent().x * 0;
				int yy = y + cube.extent().y * 1;
				gli::f32vec3 color = GetTexturePixelColor(cube, x, y, TEXTURE_CUBE_MAP_NEGATIVE_X);
				SetTexturePixelColor(texture, xx, yy, 0, color);
			}
			// TEXTURE_CUBE_MAP_POSITIVE_Y
			{
				int xx = x + cube.extent().x * 1;
				int yy = y + cube.extent().y * 0;
				gli::f32vec3 color = GetTexturePixelColor(cube, x, y, TEXTURE_CUBE_MAP_POSITIVE_Y);
				SetTexturePixelColor(texture, xx, yy, 0, color);
			}
			// TEXTURE_CUBE_MAP_NEGATIVE_Y
			{
				int xx = x + cube.extent().x * 1;
				int yy = y + cube.extent().y * 2;
				gli::f32vec3 color = GetTexturePixelColor(cube, x, y, TEXTURE_CUBE_MAP_NEGATIVE_Y);
				SetTexturePixelColor(texture, xx, yy, 0, color);
			}
			// TEXTURE_CUBE_MAP_POSITIVE_Z
			{
				int xx = x + cube.extent().x * 1;
				int yy = y + cube.extent().y * 1;
				gli::f32vec3 color = GetTexturePixelColor(cube, x, y, TEXTURE_CUBE_MAP_POSITIVE_Z);
				SetTexturePixelColor(texture, xx, yy, 0, color);
			}
			// TEXTURE_CUBE_MAP_NEGATIVE_Z
			{
				int xx = x + cube.extent().x * 3;
				int yy = y + cube.extent().y * 1;
				gli::f32vec3 color = GetTexturePixelColor(cube, x, y, TEXTURE_CUBE_MAP_NEGATIVE_Z);
				SetTexturePixelColor(texture, xx, yy, 0, color);
			}
		}
	}

	return texture;
}

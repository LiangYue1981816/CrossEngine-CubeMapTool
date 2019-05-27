#pragma once
#include "gli/gli.hpp"
#include "gli/convert.hpp"
#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtc/matrix_transform.hpp"


#define TEXTURE_CUBE_MAP_POSITIVE_X 0
#define TEXTURE_CUBE_MAP_NEGATIVE_X 1
#define TEXTURE_CUBE_MAP_POSITIVE_Y 2
#define TEXTURE_CUBE_MAP_NEGATIVE_Y 3
#define TEXTURE_CUBE_MAP_POSITIVE_Z 4
#define TEXTURE_CUBE_MAP_NEGATIVE_Z 5


template <typename texture_type>
texture_type LoadTexture(const char *szFileName)
{
	texture_type texture = (texture_type)gli::load(szFileName);
	if (texture.empty()) return texture_type();
	if (gli::is_compressed(texture.format())) return texture_type();
	return gli::convert<texture_type>(texture, gli::FORMAT_RGB32_SFLOAT_PACK32);
}

template <typename texture_type>
void SaveTextureDDS(const char *szFileName, const texture_type &texture)
{
	gli::save_dds(texture, szFileName);
}

template <typename texture_type>
void SaveTextureKTX(const char *szFileName, const texture_type &texture)
{
	gli::save_ktx(texture, szFileName);
}

template <typename texture_type>
void SaveTextureKMG(const char *szFileName, const texture_type &texture)
{
	gli::save_kmg(texture, szFileName);
}

void SetTexturePixelColor(gli::texture2d &texture, int x, int y, int level, const glm::f32vec3 &color);
void SetTexturePixelColor(gli::texture_cube &texture, int x, int y, int face, int level, const glm::f32vec3 &color);
glm::f32vec3 GetTexturePixelColor(const gli::texture2d &texture, int x, int y);
glm::f32vec3 GetTexturePixelColor(const gli::texture_cube &texture, int x, int y, int face);
glm::f32vec3 GetTexturePixelColor(const gli::texture_cube &texture, const glm::vec3 &direction);
gli::texture2d Preview(const gli::texture_cube &cube);

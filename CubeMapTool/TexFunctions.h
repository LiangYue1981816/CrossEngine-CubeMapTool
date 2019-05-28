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


gli::texture2d LoadTexture2D(const char *szFileName);
gli::texture_cube LoadTextureCube(const char *szFileName);
gli::texture2d ConvertLinearToSRGB(const gli::texture2d &texture);
gli::texture_cube ConvertLinearToSRGB(const gli::texture_cube &texture);
void SetTexturePixelColor(gli::texture2d &texture, int x, int y, int level, const glm::f32vec3 &color);
void SetTexturePixelColor(gli::texture_cube &texture, int x, int y, int face, int level, const glm::f32vec3 &color);
glm::f32vec3 GetTexturePixelColor(const gli::texture2d &texture, int x, int y);
glm::f32vec3 GetTexturePixelColor(const gli::texture_cube &texture, int x, int y, int face);
glm::f32vec3 GetTexturePixelColor(const gli::texture_cube &texture, const glm::vec3 &direction);
gli::texture2d Preview(const gli::texture_cube &cube);

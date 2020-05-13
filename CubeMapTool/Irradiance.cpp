#include "stdafx.h"


#pragma region SH
// https://cseweb.ucsd.edu/~ravir/papers/invlamb/josa.pdf
// http://cseweb.ucsd.edu/~ravir/papers/envmap/envmap.pdf

static const float a[9] = {
	// L0
	3.141593f,

	// L1
	2.094395f,
	2.094395f,
	2.094395f,

	// L2
	0.785398f,
	0.785398f,
	0.785398f,
	0.785398f,
	0.785398f,
};

static const float factors[9] = {
	// L0
	0.282095f, // Y(0,  0)

	// L1
	0.488603f, // Y(1, -1) (y)
	0.488603f, // Y(1,  0) (z)
	0.488603f, // Y(1, +1) (x)

	// L2
	1.092548f, // Y(2, -2) (x * y)
	1.092548f, // Y(2, -1) (y * z)
	0.315392f, // Y(2,  0) (z * z * 3 - 1)
	1.092548f, // Y(2, +1) (x * z)
	0.546274f, // Y(2, +2) (x * x - y * y)
};

static void SHBasis(float basis[], glm::vec3 direction)
{
	float x = direction.x;
	float y = direction.y;
	float z = direction.z;

	basis[0] = factors[0];

	basis[1] = factors[1] * y;
	basis[2] = factors[2] * z;
	basis[3] = factors[3] * x;

	basis[4] = factors[4] * (x * y);
	basis[5] = factors[5] * (y * z);
	basis[6] = factors[6] * (z * z * 3.0f - 1.0f);
	basis[7] = factors[7] * (x * z);
	basis[8] = factors[8] * (x * x - y * y);
}

static void SH(float sh_red[], float sh_grn[], float sh_blu[], float r, float g, float b, glm::vec3 direction)
{
	float basis[9] = { 0.0f };

	// Gamma to linear
	r = pow(r, 1.0f / 2.2f);
	g = pow(g, 1.0f / 2.2f);
	b = pow(b, 1.0f / 2.2f);

	// Diffuse Lambert
	r /= PI;
	g /= PI;
	b /= PI;

	SHBasis(basis, direction);

	for (int index = 0; index < 9; index++) {
		sh_red[index] += basis[index] * r;
		sh_grn[index] += basis[index] * g;
		sh_blu[index] += basis[index] * b;
	}
}
#pragma endregion

#pragma region Sampling
static float RadicalInverse(unsigned int bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10f;
}

static glm::vec2 Hammersley(unsigned int i, unsigned int n)
{
	return glm::vec2(1.0f * i / n, RadicalInverse(i));
}

static glm::vec3 Sampling(glm::vec2 xi)
{
	float phi = 2.0f * PI * xi.x;
	float theta = 2.0f * acos(sqrt(1.0f - xi.y));

	float sinphi = sin(phi);
	float cosphi = cos(phi);
	float sintheta = sin(theta);
	float costheta = cos(theta);

	return glm::vec3(sintheta * cosphi, sintheta * sinphi, costheta);
}

static glm::vec2 SphericalSampleing(glm::vec3 v)
{
	glm::vec2 invAtan = glm::vec2(1.0f / (2.0f * PI), 1.0f / (1.0f * PI));
	glm::vec2 uv = glm::vec2(atan2f(v.x, v.z), asinf(v.y));

	uv *= invAtan;
	uv += 0.5f;

	return uv;
}

static glm::vec3 SphericalToDirection(glm::vec2 uv)
{
	glm::vec2 invAtan = glm::vec2(2.0 * PI, 1.0 * PI);
	uv -= 0.5;
	uv *= invAtan;

	float x = sin(uv.x);
	float y = sin(uv.y);
	float z = cos(uv.x);
	float a = sqrt((1.0 - y * y) / (x * x + z * z));

	return glm::vec3(x * a, y, z * a);
}
#pragma endregion

#pragma region IrradianceMap
static void SaveSH(const char *szFileName, float *sh_red, float *sh_grn, float *sh_blu)
{
	if (FILE *pFile = fopen(szFileName, "wb")) {
		for (int index = 0; index < 9; index++) {
			fprintf(pFile, "%f", sh_red[index]);
			fprintf(pFile, index < 8 ? " " : "\n");
		}

		for (int index = 0; index < 9; index++) {
			fprintf(pFile, "%f", sh_grn[index]);
			fprintf(pFile, index < 8 ? " " : "\n");
		}

		for (int index = 0; index < 9; index++) {
			fprintf(pFile, "%f", sh_blu[index]);
			fprintf(pFile, index < 8 ? " " : "\n");
		}

		fclose(pFile);
	}
}

static void GenerateIrradianceEnvMapSH(const gli::texture2d &texture, float *sh_red, float *sh_grn, float *sh_blu, int samples)
{
	for (int index = 0; index < samples; index++) {
		glm::vec3 direction = glm::normalize(Sampling(Hammersley(index, samples)));
		glm::vec2 uv = SphericalSampleing(direction);
		gli::f32vec3 color = GetTexturePixelColor(texture, (int)(uv.x * (texture.extent().x - 1) + 0.5f), (int)((1.0f - uv.y) * (texture.extent().y - 1) + 0.5f));
		SH(sh_red, sh_grn, sh_blu, color.r, color.g, color.b, direction);
	}

	for (int index = 0; index < 9; index++) {
		sh_red[index] *= a[index] * factors[index] * PI * 4.0f / samples;
		sh_grn[index] *= a[index] * factors[index] * PI * 4.0f / samples;
		sh_blu[index] *= a[index] * factors[index] * PI * 4.0f / samples;
	}
}

static void GenerateIrradianceCubeMapSH(const gli::texture_cube &texture, float *sh_red, float *sh_grn, float *sh_blu, int samples)
{
	for (int index = 0; index < samples; index++) {
		glm::vec3 direction = glm::normalize(Sampling(Hammersley(index, samples)));
		gli::f32vec3 color = GetTexturePixelColor(texture, direction);
		SH(sh_red, sh_grn, sh_blu, color.r, color.g, color.b, direction);
	}

	for (int index = 0; index < 9; index++) {
		sh_red[index] *= a[index] * factors[index] * PI * 4.0f / samples;
		sh_grn[index] *= a[index] * factors[index] * PI * 4.0f / samples;
		sh_blu[index] *= a[index] * factors[index] * PI * 4.0f / samples;
	}
}

static BOOL RenderNormalizeEnvMap(const gli::texture2d &texEnvMap, gli::texture2d &texNormalizeMap, float *sh_red, float *sh_grn, float *sh_blu)
{
	static const GLchar *szShaderVertexCode =
		"                                                                                           \n\
			#version 330                                                                            \n\
																									\n\
			uniform mat4 _modelViewProjectionMatrix;                                                \n\
																									\n\
			attribute vec3 _position;                                                               \n\
			attribute vec4 _texcoord;                                                               \n\
																									\n\
			varying vec4 texcoord;                                                                  \n\
																									\n\
			void main()                                                                             \n\
			{                                                                                       \n\
				gl_Position = _modelViewProjectionMatrix*vec4(_position, 1.0);                      \n\
				texcoord = _texcoord;                                                               \n\
			}                                                                                       \n\
		";

	static const GLchar *szShaderFragmentCode =
		"                                                                                           \n\
			#version 330                                                                            \n\
																									\n\
			#define PI 3.1415926535897932384626433832795f                                           \n\
																									\n\
			uniform float _sh_red[9];                                                               \n\
			uniform float _sh_grn[9];                                                               \n\
			uniform float _sh_blu[9];                                                               \n\
																									\n\
			uniform sampler2D _envmap;                                                              \n\
																									\n\
			varying vec4 texcoord;                                                                  \n\
																									\n\
			vec3 SH(vec3 direction)                                                                 \n\
			{                                                                                       \n\
				float basis[9];                                                                     \n\
																									\n\
				float x = direction.x;                                                              \n\
				float y = direction.y;                                                              \n\
				float z = direction.z;                                                              \n\
																									\n\
				vec3 color = vec3(0.0f, 0.0f, 0.0f);                                                \n\
																									\n\
				basis[0] = 1.0f;                                                                    \n\
																									\n\
				basis[1] = y;                                                                       \n\
				basis[2] = z;                                                                       \n\
				basis[3] = x;                                                                       \n\
																									\n\
				basis[4] = (x * y);                                         						\n\
				basis[5] = (y * z);                                                                 \n\
				basis[6] = (z * z * 3.0f - 1.0f);                                                   \n\
				basis[7] = (x * z);                                                                 \n\
				basis[8] = (x * x - y * y);                                                         \n\
																									\n\
				for (int index = 0; index < 9; index++)                                             \n\
				{                                                                                   \n\
					color.r += _sh_red[index] * basis[index];                                       \n\
					color.g += _sh_grn[index] * basis[index];                                       \n\
					color.b += _sh_blu[index] * basis[index];                                       \n\
				}                                                                                   \n\
																									\n\
				return color;                                                                       \n\
			}                                                                                       \n\
																									\n\
			vec3 SphericalToDirection(vec2 uv)													    \n\
			{                                                                                       \n\
				vec2 invAtan = vec2(2.0 * PI, 1.0 * PI);                                            \n\
				uv -= 0.5;                                                                          \n\
				uv *= invAtan;                                                                      \n\
																									\n\
				float x = sin(uv.x); 																\n\
				float y = sin(uv.y); 																\n\
				float z = cos(uv.x); 																\n\
				float a = sqrt((1.0 - y * y) / (x * x + z * z)); 									\n\
																									\n\
				return vec3(x * a, y, z * a);                                                       \n\
			}                                                                                       \n\
																									\n\
			void main()                                                                             \n\
			{                                                                                       \n\
				vec3 direction = SphericalToDirection(texcoord.xy);                                 \n\
				direction = normalize(direction);                                                   \n\
																									\n\
				vec3 sh = SH(direction.xyz);														\n\
				vec2 uv = vec2(texcoord.x, 1.0f - texcoord.y);										\n\
				vec3 color = pow(texture(_envmap, uv).rgb, vec3(1.0f / 2.2f));						\n\
																									\n\
				gl_FragColor.rgb = color / sh;														\n\
				gl_FragColor.rgb = pow(gl_FragColor.rgb, vec3(2.2f));                               \n\
			}                                                                                       \n\
		";

	static const vertex vertices[4] = {
		{ { -1.0f, -1.0f, 0.0f },{ 0.0f, 0.0f } },
		{ {  1.0f, -1.0f, 0.0f },{ 1.0f, 0.0f } },
		{ {  1.0f,  1.0f, 0.0f },{ 1.0f, 1.0f } },
		{ { -1.0f,  1.0f, 0.0f },{ 0.0f, 1.0f } },
	};
	static const unsigned short indices[6] = { 0, 1, 2, 2, 3, 0 };

	BOOL rcode = TRUE;

	gli::gl GL(gli::gl::PROFILE_ES30);
	gli::gl::format glFormat = GL.translate(texNormalizeMap.format());

	GLuint texture = 0;
	if (GLCreateTexture2D(texEnvMap, texture) == FALSE) goto ERR;
	if (GLCreateVBO(vertices, 4, indices, 6) == FALSE) goto ERR;
	if (GLCreateProgram(szShaderVertexCode, szShaderFragmentCode) == FALSE) goto ERR;
	{
		if (GLCreateFBO(texNormalizeMap.extent().x, texNormalizeMap.extent().y, texNormalizeMap.format()) == FALSE) goto ERR;
		{
			glEnable(GL_TEXTURE_2D);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			{
				glm::mat4 matModeView = glm::lookAt(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
				glm::mat4 matProjection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
				glm::mat4 matModeViewProjection = matProjection * matModeView;

				glViewport(0, 0, texNormalizeMap.extent().x, texNormalizeMap.extent().y);
				glUseProgram(program);
				glBindFramebuffer(GL_FRAMEBUFFER, fbo);
				glBindBuffer(GL_ARRAY_BUFFER, vbo);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
				glEnableVertexAttribArray(attribLocationPosition);
				glEnableVertexAttribArray(attribLocationTexcoord);
				{
					glVertexAttribPointer(attribLocationPosition, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (GLvoid *)0);
					glVertexAttribPointer(attribLocationTexcoord, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (GLvoid *)12);

					glUniformMatrix4fv(uniformLocationModelViewProjectionMatrix, 1, GL_FALSE, (const float *)&matModeViewProjection);
					glUniform1fv(uniformLocationSHRed, 9, sh_red);
					glUniform1fv(uniformLocationSHGrn, 9, sh_grn);
					glUniform1fv(uniformLocationSHBlu, 9, sh_blu);
					glUniform1i(uniformLocationEnvmap, 0);

					glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
					glReadPixels(0, 0, texNormalizeMap.extent().x, texNormalizeMap.extent().y, glFormat.External, glFormat.Type, texNormalizeMap.data(0, 0, 0));
				}
				glDisableVertexAttribArray(attribLocationPosition);
				glDisableVertexAttribArray(attribLocationTexcoord);
				glBindBuffer(GL_ARRAY_BUFFER, 0);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				glUseProgram(0);
			}
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		}
		GLDestroyFBO();
	}

	texNormalizeMap = gli::flip(texNormalizeMap);

	goto RET;
ERR:
	rcode = FALSE;
RET:
	GLDestroyVBO();
	GLDestroyFBO();
	GLDestroyProgram();
	GLDestroyTexture(texture);

	return rcode;
}

static BOOL RenderNormalizeCubeMap(const gli::texture_cube &texCubeMap, gli::texture_cube &texNormalizeMap, float *sh_red, float *sh_grn, float *sh_blu)
{
	static const GLchar *szShaderVertexCode =
		"                                                                                           \n\
			#version 330                                                                            \n\
																									\n\
			uniform mat4 _modelViewProjectionMatrix;                                                \n\
																									\n\
			attribute vec3 _position;                                                               \n\
			attribute vec4 _texcoord;                                                               \n\
																									\n\
			varying vec4 texcoord;                                                                  \n\
																									\n\
			void main()                                                                             \n\
			{                                                                                       \n\
				gl_Position = _modelViewProjectionMatrix*vec4(_position, 1.0);                      \n\
				texcoord = _texcoord;                                                               \n\
			}                                                                                       \n\
		";

	static const GLchar *szShaderFragmentCode =
		"                                                                                           \n\
			#version 330                                                                            \n\
																									\n\
			#define PI 3.1415926535897932384626433832795f                                           \n\
																									\n\
			uniform float _sh_red[9];                                                               \n\
			uniform float _sh_grn[9];                                                               \n\
			uniform float _sh_blu[9];                                                               \n\
																									\n\
			uniform samplerCube _cubemap;                                                           \n\
			uniform mat4 _texcoordMatrix;                                                           \n\
																									\n\
			varying vec4 texcoord;                                                                  \n\
																									\n\
			vec3 SH(vec3 direction)                                                                 \n\
			{                                                                                       \n\
				float basis[9];                                                                     \n\
																									\n\
				float x = direction.x;                                                              \n\
				float y = direction.y;                                                              \n\
				float z = direction.z;                                                              \n\
																									\n\
				vec3 color = vec3(0.0f, 0.0f, 0.0f);                                                \n\
																									\n\
				basis[0] = 1.0f;                                                                    \n\
																									\n\
				basis[1] = y;                                                                       \n\
				basis[2] = z;                                                                       \n\
				basis[3] = x;                                                                       \n\
																									\n\
				basis[4] = (x * y);                                         						\n\
				basis[5] = (y * z);                                                                 \n\
				basis[6] = (z * z * 3.0f - 1.0f);                                                   \n\
				basis[7] = (x * z);                                                                 \n\
				basis[8] = (x * x - y * y);                                                         \n\
																									\n\
				for (int index = 0; index < 9; index++)                                             \n\
				{                                                                                   \n\
					color.r += _sh_red[index] * basis[index];                                       \n\
					color.g += _sh_grn[index] * basis[index];                                       \n\
					color.b += _sh_blu[index] * basis[index];                                       \n\
				}                                                                                   \n\
																									\n\
				return color;                                                                       \n\
			}                                                                                       \n\
																									\n\
			void main()                                                                             \n\
			{                                                                                       \n\
				vec4 direction = _texcoordMatrix * vec4(texcoord.x, texcoord.y, 1.0f, 0.0f);        \n\
				direction = normalize(direction);                                                   \n\
																									\n\
				vec3 sh = SH(direction.xyz);														\n\
				vec3 color = pow(texture(_cubemap, direction).rgb, vec3(1.0f / 2.2f));				\n\
																									\n\
				gl_FragColor.rgb = color / sh;														\n\
				gl_FragColor.rgb = pow(gl_FragColor.rgb, vec3(2.2f));                               \n\
			}                                                                                       \n\
		";
	
	static const vertex vertices[4] = {
		{ { -1.0f, -1.0f, 0.0f },{ -1.0f, -1.0f } },
		{ {  1.0f, -1.0f, 0.0f },{  1.0f, -1.0f } },
		{ {  1.0f,  1.0f, 0.0f },{  1.0f,  1.0f } },
		{ { -1.0f,  1.0f, 0.0f },{ -1.0f,  1.0f } },
	};
	static const unsigned short indices[6] = { 0, 1, 2, 2, 3, 0 };

	BOOL rcode = TRUE;

	gli::gl GL(gli::gl::PROFILE_ES30);
	gli::gl::format glFormat = GL.translate(texNormalizeMap.format());

	GLuint texture = 0;
	if (GLCreateTextureCube(texCubeMap, texture) == FALSE) goto ERR;
	if (GLCreateVBO(vertices, 4, indices, 6) == FALSE) goto ERR;
	if (GLCreateProgram(szShaderVertexCode, szShaderFragmentCode) == FALSE) goto ERR;
	{
		if (GLCreateFBO(texNormalizeMap.extent().x, texNormalizeMap.extent().y, texNormalizeMap.format()) == FALSE) goto ERR;
		{
			glEnable(GL_TEXTURE_CUBE_MAP);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			{
				glm::mat4 matModeView = glm::lookAt(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
				glm::mat4 matProjection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
				glm::mat4 matModeViewProjection = matProjection * matModeView;
				glm::mat4 matTexcoords[6] = {
					glm::rotate(glm::mat4(),  PI / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f)),
					glm::rotate(glm::mat4(), -PI / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f)),
					glm::rotate(glm::mat4(), -PI / 2.0f, glm::vec3(1.0f, 0.0f, 0.0f)),
					glm::rotate(glm::mat4(),  PI / 2.0f, glm::vec3(1.0f, 0.0f, 0.0f)),
					glm::mat4(),
					glm::rotate(glm::mat4(),  PI, glm::vec3(0.0f, 1.0f, 0.0f)),
				};

				glViewport(0, 0, texNormalizeMap.extent().x, texNormalizeMap.extent().y);
				glUseProgram(program);
				glBindFramebuffer(GL_FRAMEBUFFER, fbo);
				glBindBuffer(GL_ARRAY_BUFFER, vbo);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
				glEnableVertexAttribArray(attribLocationPosition);
				glEnableVertexAttribArray(attribLocationTexcoord);
				{
					glVertexAttribPointer(attribLocationPosition, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (GLvoid *)0);
					glVertexAttribPointer(attribLocationTexcoord, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (GLvoid *)12);

					glUniformMatrix4fv(uniformLocationModelViewProjectionMatrix, 1, GL_FALSE, (const float *)&matModeViewProjection);
					glUniform1fv(uniformLocationSHRed, 9, sh_red);
					glUniform1fv(uniformLocationSHGrn, 9, sh_grn);
					glUniform1fv(uniformLocationSHBlu, 9, sh_blu);
					glUniform1i(uniformLocationCubemap, 0);

					for (int index = 0; index < 6; index++)
					{
						glUniformMatrix4fv(uniformLocationTexcoordMatrix, 1, GL_FALSE, (const float *)&matTexcoords[index]);
						glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
						glReadPixels(0, 0, texNormalizeMap.extent().x, texNormalizeMap.extent().y, glFormat.External, glFormat.Type, texNormalizeMap.data(0, index, 0));
					}
				}
				glDisableVertexAttribArray(attribLocationPosition);
				glDisableVertexAttribArray(attribLocationTexcoord);
				glBindBuffer(GL_ARRAY_BUFFER, 0);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				glUseProgram(0);
			}
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		}
		GLDestroyFBO();
	}

	texNormalizeMap = gli::flip(texNormalizeMap);

	goto RET;
ERR:
	rcode = FALSE;
RET:
	GLDestroyVBO();
	GLDestroyFBO();
	GLDestroyProgram();
	GLDestroyTexture(texture);

	return rcode;
}

static BOOL RenderIrradianceMap(gli::texture_cube &texture, float *sh_red, float *sh_grn, float *sh_blu)
{
	static const GLchar *szShaderVertexCode =
		"                                                                                           \n\
			#version 330                                                                            \n\
																									\n\
			uniform mat4 _modelViewProjectionMatrix;                                                \n\
																									\n\
			attribute vec3 _position;                                                               \n\
			attribute vec4 _texcoord;                                                               \n\
																									\n\
			varying vec4 texcoord;                                                                  \n\
																									\n\
			void main()                                                                             \n\
			{                                                                                       \n\
				gl_Position = _modelViewProjectionMatrix*vec4(_position, 1.0);                      \n\
				texcoord = _texcoord;                                                               \n\
			}                                                                                       \n\
		";

	static const GLchar *szShaderFragmentCode =
		"                                                                                           \n\
			#version 330                                                                            \n\
																									\n\
			uniform float _sh_red[9];                                                               \n\
			uniform float _sh_grn[9];                                                               \n\
			uniform float _sh_blu[9];                                                               \n\
																									\n\
			uniform mat4 _texcoordMatrix;                                                           \n\
																									\n\
			varying vec4 texcoord;                                                                  \n\
																									\n\
			vec3 SH(vec3 direction)                                                                 \n\
			{                                                                                       \n\
				float basis[9];                                                                     \n\
																									\n\
				float x = direction.x;                                                              \n\
				float y = direction.y;                                                              \n\
				float z = direction.z;                                                              \n\
																									\n\
				vec3 color = vec3(0.0f, 0.0f, 0.0f);                                                \n\
																									\n\
				basis[0] = 1.0f;                                                                    \n\
																									\n\
				basis[1] = y;                                                                       \n\
				basis[2] = z;                                                                       \n\
				basis[3] = x;                                                                       \n\
																									\n\
				basis[4] = (x * y);                                         						\n\
				basis[5] = (y * z);                                                                 \n\
				basis[6] = (z * z * 3.0f - 1.0f);                                                   \n\
				basis[7] = (x * z);                                                                 \n\
				basis[8] = (x * x - y * y);                                                         \n\
																									\n\
				for (int index = 0; index < 9; index++)                                             \n\
				{                                                                                   \n\
					color.r += _sh_red[index] * basis[index];                                       \n\
					color.g += _sh_grn[index] * basis[index];                                       \n\
					color.b += _sh_blu[index] * basis[index];                                       \n\
				}                                                                                   \n\
																									\n\
				return color;                                                                       \n\
			}                                                                                       \n\
																									\n\
			void main()                                                                             \n\
			{                                                                                       \n\
				vec4 direction = _texcoordMatrix * vec4(texcoord.x, texcoord.y, 1.0f, 0.0f);        \n\
				direction.xyz = normalize(direction.xyz);                                           \n\
																									\n\
				gl_FragColor.rgb = SH(direction.xyz);                                               \n\
				gl_FragColor.rgb = pow(gl_FragColor.rgb, vec3(2.2f));                               \n\
			}                                                                                       \n\
		";

	static const vertex vertices[4] = {
		{ { -1.0f, -1.0f, 0.0f },{ -1.0f, -1.0f } },
		{ {  1.0f, -1.0f, 0.0f },{  1.0f, -1.0f } },
		{ {  1.0f,  1.0f, 0.0f },{  1.0f,  1.0f } },
		{ { -1.0f,  1.0f, 0.0f },{ -1.0f,  1.0f } },
	};
	static const unsigned short indices[6] = { 0, 1, 2, 2, 3, 0 };

	BOOL rcode = TRUE;

	gli::gl GL(gli::gl::PROFILE_ES30);
	gli::gl::format glFormat = GL.translate(texture.format());

	if (GLCreateVBO(vertices, 4, indices, 6) == FALSE) goto ERR;
	if (GLCreateFBO(texture.extent().x, texture.extent().y, texture.format()) == FALSE) goto ERR;
	if (GLCreateProgram(szShaderVertexCode, szShaderFragmentCode) == FALSE) goto ERR;
	{
		glm::mat4 matModeView = glm::lookAt(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 matProjection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
		glm::mat4 matModeViewProjection = matProjection * matModeView;
		glm::mat4 matTexcoords[6] = {
			glm::rotate(glm::mat4(),  PI / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f)),
			glm::rotate(glm::mat4(), -PI / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f)),
			glm::rotate(glm::mat4(), -PI / 2.0f, glm::vec3(1.0f, 0.0f, 0.0f)),
			glm::rotate(glm::mat4(),  PI / 2.0f, glm::vec3(1.0f, 0.0f, 0.0f)),
			glm::mat4(),
			glm::rotate(glm::mat4(),  PI, glm::vec3(0.0f, 1.0f, 0.0f)),
		};

		glViewport(0, 0, texture.extent().x, texture.extent().y);
		glUseProgram(program);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glEnableVertexAttribArray(attribLocationPosition);
		glEnableVertexAttribArray(attribLocationTexcoord);
		{
			glVertexAttribPointer(attribLocationPosition, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (GLvoid *)0);
			glVertexAttribPointer(attribLocationTexcoord, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (GLvoid *)12);

			glUniformMatrix4fv(uniformLocationModelViewProjectionMatrix, 1, GL_FALSE, (const float *)&matModeViewProjection);
			glUniform1fv(uniformLocationSHRed, 9, sh_red);
			glUniform1fv(uniformLocationSHGrn, 9, sh_grn);
			glUniform1fv(uniformLocationSHBlu, 9, sh_blu);

			for (int index = 0; index < 6; index++)
			{
				glUniformMatrix4fv(uniformLocationTexcoordMatrix, 1, GL_FALSE, (const float *)&matTexcoords[index]);
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
				glReadPixels(0, 0, texture.extent().x, texture.extent().y, glFormat.External, glFormat.Type, texture.data(0, index, 0));
			}
		}
		glDisableVertexAttribArray(attribLocationPosition);
		glDisableVertexAttribArray(attribLocationTexcoord);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glUseProgram(0);
	}

	texture = gli::flip(texture);

	goto RET;
ERR:
	rcode = FALSE;
RET:
	GLDestroyVBO();
	GLDestroyFBO();
	GLDestroyProgram();

	return rcode;
}

BOOL GenerateEnvIrradianceMap(gli::texture2d &texEnvMap, gli::texture_cube &texIrrMap, int samples)
{
	float sh_red[9] = { 0.0f };
	float sh_grn[9] = { 0.0f };
	float sh_blu[9] = { 0.0f };

	GenerateIrradianceEnvMapSH(texEnvMap, sh_red, sh_grn, sh_blu, samples);
	RenderIrradianceMap(texIrrMap, sh_red, sh_grn, sh_blu);
	SaveSH("IrradianceSH.output", sh_red, sh_grn, sh_blu);

	//gli::texture2d texNormalizeMap(texEnvMap.format(), texEnvMap.extent());
	//RenderNormalizeEnvMap(texEnvMap, texNormalizeMap, sh_red, sh_grn, sh_blu);
	//gli::save_dds(texNormalizeMap, "result.dds");

	return TRUE;
}

BOOL GenerateCubeIrradianceMap(gli::texture_cube &texCubeMap, gli::texture_cube &texIrrMap, int samples)
{
	float sh_red[9] = { 0.0f };
	float sh_grn[9] = { 0.0f };
	float sh_blu[9] = { 0.0f };

	GenerateIrradianceCubeMapSH(texCubeMap, sh_red, sh_grn, sh_blu, samples);
	RenderIrradianceMap(texIrrMap, sh_red, sh_grn, sh_blu);
	SaveSH("IrradianceSH.output", sh_red, sh_grn, sh_blu);

	//gli::texture2d texNormalizeMap(texEnvMap.format(), texEnvMap.extent());
	//RenderNormalizeEnvMap(texEnvMap, texNormalizeMap, sh_red, sh_grn, sh_blu);
	//gli::save_dds(texNormalizeMap, "result.dds");

	return TRUE;
}
#pragma endregion

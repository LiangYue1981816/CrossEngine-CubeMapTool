#include "stdafx.h"


BOOL GenerateEnvMipmaps(gli::texture2d &texEnvMap, gli::texture2d &texEnvMipmap, int samples)
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
			uniform uint _samples;                                                                  \n\
			uniform float _roughness;                                                               \n\
			uniform sampler2D _envmap;                                                              \n\
																									\n\
			uniform float _sh_red[9];                                                               \n\
			uniform float _sh_grn[9];                                                               \n\
			uniform float _sh_blu[9];                                                               \n\
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
			float RadicalInverse(uint bits)                                                         \n\
			{                                                                                       \n\
				bits = (bits << 16u) | (bits >> 16u);                                               \n\
				bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);                 \n\
				bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);                 \n\
				bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);                 \n\
				bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);                 \n\
				return float(bits) * 2.3283064365386963e-10;                                        \n\
			}                                                                                       \n\
																									\n\
			vec2 Hammersley(uint i, uint n)                                                         \n\
			{                                                                                       \n\
				return vec2(1.0f * i / n, RadicalInverse(i));                                       \n\
			}                                                                                       \n\
																									\n\
			vec2 SphericalSampleing(vec3 v) 														\n\
			{                                                                                       \n\
				vec2 invAtan = vec2(1.0 / (2.0 * PI), 1.0 / (1.0 * PI));                            \n\
				vec2 uv = vec2(atan(v.x, v.z), -asin(v.y));                                         \n\
																									\n\
				uv *= invAtan;                                                                      \n\
				uv += 0.5;                                                                          \n\
																									\n\
				return uv;                                                                          \n\
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
			vec3 ImportanceSamplingGGX(vec2 xi, vec3 normal, float roughness)                       \n\
			{                                                                                       \n\
				float a = roughness * roughness;                                                    \n\
																									\n\
				float phi = 2.0f * PI * xi.x;                                                       \n\
				float costheta = sqrt((1.0f - xi.y) / (1.0f + (a * a - 1.0f) * xi.y));              \n\
				float sintheta = sqrt(1.0f - costheta * costheta);                                  \n\
																									\n\
				vec3 h = vec3(sintheta * cos(phi), sintheta * sin(phi), costheta);                  \n\
				vec3 up = abs(normal.z) < 0.999f ? vec3(0.0f, 0.0f, 1.0f) : vec3(1.0f, 0.0f, 0.0f); \n\
				vec3 tx = normalize(cross(up, normal));                                             \n\
				vec3 ty = cross(normal, tx);                                                        \n\
																									\n\
				return normalize(tx * h.x + ty * h.y + normal * h.z);                               \n\
			}                                                                                       \n\
																									\n\
			vec3 Sampling(sampler2D envmap, vec3 normal, float roughness, uint samples)             \n\
			{                                                                                       \n\
				vec3 N = normal;                                                                    \n\
				vec3 V = normal;                                                                    \n\
																									\n\
				float weight = 0.0f;                                                                \n\
				vec3 color = vec3(0.0f, 0.0f, 0.0f);                                                \n\
																									\n\
				for (uint index = 0u; index < samples; index++)                                     \n\
				{                                                                                   \n\
					vec2 Xi = Hammersley(index, samples);                                           \n\
					vec3 H = ImportanceSamplingGGX(Xi, N, roughness);                               \n\
					vec3 L = normalize(dot(V, H) * H * 2.0f - V);                                   \n\
					vec2 uv = SphericalSampleing(L);                                                \n\
																									\n\
					float ndotl = max(dot(N, L), 0.0f);                                             \n\
																									\n\
					if (ndotl > 0.0f) {                                                             \n\
						color += pow(texture(envmap, uv).rgb, vec3(1.0f / 2.2f)) * ndotl;           \n\
						weight += ndotl;                                                            \n\
					}                                                                               \n\
				}                                                                                   \n\
																									\n\
				color /= weight;                                                                    \n\
				return color;                                                                       \n\
			}                                                                                       \n\
																									\n\
			void main()                                                                             \n\
			{                                                                                       \n\
				vec3 direction = SphericalToDirection(texcoord.xy);                                 \n\
				direction = normalize(direction);                                                   \n\
				gl_FragColor.rgb = Sampling(_envmap, direction, _roughness, _samples);              \n\
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
	gli::gl::format glFormat = GL.translate(texEnvMipmap.format());

	GLuint texture = 0;
	if (GLCreateTexture2D(texEnvMap, texture) == FALSE) goto ERR;
	if (GLCreateVBO(vertices, 4, indices, 6) == FALSE) goto ERR;
	if (GLCreateProgram(szShaderVertexCode, szShaderFragmentCode) == FALSE) goto ERR;
	{
		for (int mipLevel = 0; mipLevel < (int)texEnvMipmap.levels(); mipLevel++) {
			if (GLCreateFBO(texEnvMipmap.extent(mipLevel).x, texEnvMipmap.extent(mipLevel).y, texEnvMipmap.format()) == FALSE) goto ERR;
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

					glViewport(0, 0, texEnvMipmap.extent(mipLevel).x, texEnvMipmap.extent(mipLevel).y);
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
						glUniform1f(uniformLocationRoughness, mipLevel / (texEnvMipmap.levels() - 1.0f));
						glUniform1ui(uniformLocationSamples, samples);
						glUniform1i(uniformLocationEnvmap, 0);

						glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
						glReadPixels(0, 0, texEnvMipmap.extent(mipLevel).x, texEnvMipmap.extent(mipLevel).y, glFormat.External, glFormat.Type, texEnvMipmap.data(0, 0, mipLevel));
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
	}

	texEnvMipmap = gli::flip(texEnvMipmap);

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

BOOL GenerateCubeMipmaps(gli::texture_cube &texCubeMap, gli::texture_cube &texCubeMipmap, int samples)
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
			uniform uint _samples;                                                                  \n\
			uniform float _roughness;                                                               \n\
			uniform samplerCube _cubemap;                                                           \n\
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
			float RadicalInverse(uint bits)                                                         \n\
			{                                                                                       \n\
				bits = (bits << 16u) | (bits >> 16u);                                               \n\
				bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);                 \n\
				bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);                 \n\
				bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);                 \n\
				bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);                 \n\
				return float(bits) * 2.3283064365386963e-10;                                        \n\
			}                                                                                       \n\
																									\n\
			vec2 Hammersley(uint i, uint n)                                                         \n\
			{                                                                                       \n\
				return vec2(1.0f * i / n, RadicalInverse(i));                                       \n\
			}                                                                                       \n\
																									\n\
			vec3 ImportanceSamplingGGX(vec2 xi, vec3 normal, float roughness)                       \n\
			{                                                                                       \n\
				float a = roughness * roughness;                                                    \n\
																									\n\
				float phi = 2.0f * PI * xi.x;                                                       \n\
				float costheta = sqrt((1.0f - xi.y) / (1.0f + (a * a - 1.0f) * xi.y));              \n\
				float sintheta = sqrt(1.0f - costheta * costheta);                                  \n\
																									\n\
				vec3 h = vec3(sintheta * cos(phi), sintheta * sin(phi), costheta);                  \n\
				vec3 up = abs(normal.z) < 0.999f ? vec3(0.0f, 0.0f, 1.0f) : vec3(1.0f, 0.0f, 0.0f); \n\
				vec3 tx = normalize(cross(up, normal));                                             \n\
				vec3 ty = cross(normal, tx);                                                        \n\
																									\n\
				return normalize(tx * h.x + ty * h.y + normal * h.z);                               \n\
			}                                                                                       \n\
																									\n\
			vec3 Sampling(samplerCube cubemap, vec3 normal, float roughness, uint samples)          \n\
			{                                                                                       \n\
				vec3 N = normal;                                                                    \n\
				vec3 V = normal;                                                                    \n\
																									\n\
				float weight = 0.0f;                                                                \n\
				vec3 color = vec3(0.0f, 0.0f, 0.0f);                                                \n\
																									\n\
				for (uint index = 0u; index < samples; index++)                                     \n\
				{                                                                                   \n\
					vec2 Xi = Hammersley(index, samples);                                           \n\
					vec3 H = ImportanceSamplingGGX(Xi, N, roughness);                               \n\
					vec3 L = normalize(dot(V, H) * H * 2.0f - V);                                   \n\
																									\n\
					float ndotl = max(dot(N, L), 0.0f);                                             \n\
																									\n\
					if (ndotl > 0.0f) {                                                             \n\
						color += pow(texture(cubemap, L).rgb, vec3(1.0f / 2.2f)) * ndotl;           \n\
						weight += ndotl;                                                            \n\
					}                                                                               \n\
				}                                                                                   \n\
																									\n\
				color /= weight;                                                                    \n\
				return color;                                                                       \n\
			}                                                                                       \n\
																									\n\
			void main()                                                                             \n\
			{                                                                                       \n\
				vec4 direction = _texcoordMatrix * vec4(texcoord.x, texcoord.y, 1.0f, 0.0f);        \n\
				direction.xyz = normalize(direction.xyz);                                           \n\
				gl_FragColor.rgb = Sampling(_cubemap, direction.xyz, _roughness, _samples);         \n\
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
	gli::gl::format glFormat = GL.translate(texCubeMipmap.format());

	GLuint texture = 0;
	if (GLCreateTextureCube(texCubeMap, texture) == FALSE) goto ERR;
	if (GLCreateVBO(vertices, 4, indices, 6) == FALSE) goto ERR;
	if (GLCreateProgram(szShaderVertexCode, szShaderFragmentCode) == FALSE) goto ERR;
	{
		for (int mipLevel = 0; mipLevel < (int)texCubeMipmap.levels(); mipLevel++) {
			if (GLCreateFBO(texCubeMipmap.extent(mipLevel).x, texCubeMipmap.extent(mipLevel).y, texCubeMipmap.format()) == FALSE) goto ERR;
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

					glViewport(0, 0, texCubeMipmap.extent(mipLevel).x, texCubeMipmap.extent(mipLevel).y);
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
						glUniform1f(uniformLocationRoughness, mipLevel / (texCubeMipmap.levels() - 1.0f));
						glUniform1ui(uniformLocationSamples, samples);
						glUniform1i(uniformLocationCubemap, 0);

						for (int index = 0; index < 6; index++)
						{
							glUniformMatrix4fv(uniformLocationTexcoordMatrix, 1, GL_FALSE, (const float *)&matTexcoords[index]);
							glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
							glReadPixels(0, 0, texCubeMipmap.extent(mipLevel).x, texCubeMipmap.extent(mipLevel).y, glFormat.External, glFormat.Type, texCubeMipmap.data(0, index, mipLevel));
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
	}

	texCubeMipmap = gli::flip(texCubeMipmap);

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

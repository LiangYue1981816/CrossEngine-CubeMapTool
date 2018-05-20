#include "stdafx.h"


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
			uniform samplerCube _envmap;                                                            \n\
																									\n\
			uniform mat4 _texcoordMatrix;                                                           \n\
																									\n\
			varying vec4 texcoord;                                                                  \n\
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
			vec3 Sampling(samplerCube envmap, vec3 normal, float roughness, uint samples)           \n\
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
						color += pow(texture(envmap, L).rgb, vec3(1.0f / 2.2f)) * ndotl;            \n\
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
				gl_FragColor.rgb = Sampling(_envmap, direction.xyz, _roughness, _samples);          \n\
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

BOOL GenerateCubeMipmaps(CUBEMAP *pCubeMap, CUBEMAP pMipmaps[], int mipLevels, int samples)
{
	BOOL rcode = TRUE;
	GLuint texture = CreateTextureCube(pCubeMap);

	if (CreateVBO(vertices, 4, indices, 6) == FALSE) goto ERR;
	if (CreateProgram(szShaderVertexCode, szShaderFragmentCode) == FALSE) goto ERR;
	{
		for (int mipLevel = 0; mipLevel < mipLevels; mipLevel++) {
			if (CreateFBO(CUBEMAP_WIDTH(&pMipmaps[mipLevel]), CUBEMAP_HEIGHT(&pMipmaps[mipLevel])) == FALSE) goto ERR;
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

					glViewport(0, 0, CUBEMAP_WIDTH(&pMipmaps[mipLevel]), CUBEMAP_HEIGHT(&pMipmaps[mipLevel]));
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
						glUniform1f(uniformLocationRoughness, mipLevel / (mipLevels - 1.0f));
						glUniform1ui(uniformLocationSamples, samples);
						glUniform1i(uniformLocationEnvmap, 0);

						for (int index = 0; index < 6; index++)
						{
							glUniformMatrix4fv(uniformLocationTexcoordMatrix, 1, GL_FALSE, (const float *)&matTexcoords[index]);
							glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
							glReadPixels(0, 0, CUBEMAP_WIDTH(&pMipmaps[mipLevel]), CUBEMAP_HEIGHT(&pMipmaps[mipLevel]), GL_BGR, GL_UNSIGNED_BYTE, pMipmaps[mipLevel].faces[index].data);
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
			DestroyFBO();
		}
	}

	goto RET;
ERR:
	rcode = FALSE;
RET:
	DestroyVBO();
	DestroyFBO();
	DestroyProgram();
	DestroyTexture(texture);

	return rcode;
}

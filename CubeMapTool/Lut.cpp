#include "stdafx.h"


BOOL GenerateLutMap(gli::texture2d &texLutMap, int samples)
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
		#version 330                                                                                \n\
																									\n\
		#define PI 3.1415926535897932384626433832795f                                               \n\
																									\n\
		uniform uint _samples;                                                                      \n\
																									\n\
		varying vec4 texcoord;                                                                      \n\
																									\n\
		float RadicalInverse(uint bits)                                                             \n\
		{                                                                                           \n\
			bits = (bits << 16u) | (bits >> 16u);                                                   \n\
			bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);                     \n\
			bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);                     \n\
			bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);                     \n\
			bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);                     \n\
			return float(bits) * 2.3283064365386963e-10;                                            \n\
		}                                                                                           \n\
																									\n\
		vec2 Hammersley(uint i, uint n)                                                             \n\
		{                                                                                           \n\
			return vec2(1.0f * i / n, RadicalInverse(i));                                           \n\
		}                                                                                           \n\
																									\n\
		vec3 ImportanceSamplingGGX(vec2 xi, vec3 normal, float roughness)                           \n\
		{                                                                                           \n\
			float a = roughness * roughness;                                                        \n\
																									\n\
			float phi = 2.0f * PI * xi.x;                                                           \n\
			float costheta = sqrt((1.0f - xi.y) / (1.0f + (a * a - 1.0f) * xi.y));                  \n\
			float sintheta = sqrt(1.0f - costheta * costheta);                                      \n\
																									\n\
			vec3 h = vec3(sintheta * cos(phi), sintheta * sin(phi), costheta);                      \n\
			vec3 up = abs(normal.z) < 0.999f ? vec3(0.0f, 0.0f, 1.0f) : vec3(1.0f, 0.0f, 0.0f);     \n\
			vec3 tx = normalize(cross(up, normal));                                                 \n\
			vec3 ty = cross(normal, tx);                                                            \n\
																									\n\
			return normalize(tx * h.x + ty * h.y + normal * h.z);                                   \n\
		}                                                                                           \n\
																									\n\
		float GeometrySchlickGGX(float roughness, float ndotv)                                      \n\
		{                                                                                           \n\
			float k = roughness * roughness / 2.0f;                                                 \n\
			return ndotv / (ndotv * (1.0f - k) + k);                                                \n\
		}                                                                                           \n\
																									\n\
		float GeometrySmith(float roughness, float ndotl, float ndotv)                              \n\
		{                                                                                           \n\
			float ggx1 = GeometrySchlickGGX(roughness, ndotl);                                      \n\
			float ggx2 = GeometrySchlickGGX(roughness, ndotv);                                      \n\
			return ggx1 * ggx2;                                                                     \n\
		}                                                                                           \n\
																									\n\
		vec2 IntegrateBRDF(float roughness, float ndotv, uint samples)                              \n\
		{                                                                                           \n\
			vec3 N = vec3(0.0f, 1.0f, 0.0f);                                                        \n\
			vec3 V = vec3(sqrt(1.0f - ndotv * ndotv), ndotv, 0.0f);                                 \n\
																									\n\
			float A = 0.0f;                                                                         \n\
			float B = 0.0f;                                                                         \n\
																									\n\
			for (uint index = 0u; index < samples; index++)                                         \n\
			{                                                                                       \n\
				vec2 Xi = Hammersley(index, samples);                                               \n\
				vec3 H = ImportanceSamplingGGX(Xi, N, roughness);                                   \n\
				vec3 L = normalize(dot(V, H) * H * 2.0f - V);                                       \n\
																									\n\
				float ndotl = max(dot(N, L), 0.0f);                                                 \n\
				float ndoth = max(dot(N, H), 0.0f);                                                 \n\
				float vdoth = max(dot(V, H), 0.0f);                                                 \n\
																									\n\
				if (ndotl > 0.0f) {                                                                 \n\
					float fc = pow(1.0f - vdoth, 5.0f);                                             \n\
					float ggx = GeometrySmith(roughness, ndotl, ndotv);                             \n\
					float vis = (ggx * vdoth) / (ndotv * ndoth);                                    \n\
					A += vis * (1.0f - fc);                                                         \n\
					B += vis * fc;                                                                  \n\
				}                                                                                   \n\
			}                                                                                       \n\
																									\n\
			A /= samples;                                                                           \n\
			B /= samples;                                                                           \n\
																									\n\
			return vec2(A, B);                                                                      \n\
		}                                                                                           \n\
																									\n\
		void main()                                                                                 \n\
		{                                                                                           \n\
			float ndotv = texcoord.x;                                                               \n\
			float roughness = texcoord.y;                                                           \n\
			gl_FragColor.rg = IntegrateBRDF(roughness, ndotv, _samples);                            \n\
			gl_FragColor.ba = vec2(0.0, 1.0);                                                       \n\
		}                                                                                           \n\
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
	gli::gl::format glFormat = GL.translate(texLutMap.format());

	if (GLCreateVBO(vertices, 4, indices, 6) == FALSE) goto ERR;
	if (GLCreateFBO(texLutMap.extent().x, texLutMap.extent().y, texLutMap.format()) == FALSE) goto ERR;
	if (GLCreateProgram(szShaderVertexCode, szShaderFragmentCode) == FALSE) goto ERR;
	{
		glm::mat4 matModeView = glm::lookAt(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 matProjection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
		glm::mat4 matModeViewProjection = matProjection * matModeView;

		glViewport(0, 0, texLutMap.extent().x, texLutMap.extent().y);
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
			glUniform1ui(uniformLocationSamples, samples);

			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
			glReadPixels(0, 0, texLutMap.extent().x, texLutMap.extent().y, glFormat.External, glFormat.Type, texLutMap.data(0, 0, 0));
		}
		glDisableVertexAttribArray(attribLocationPosition);
		glDisableVertexAttribArray(attribLocationTexcoord);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glUseProgram(0);
	}

	texLutMap = gli::flip(texLutMap);

	goto RET;
ERR:
	rcode = FALSE;
RET:
	GLDestroyVBO();
	GLDestroyFBO();
	GLDestroyProgram();

	return rcode;
}

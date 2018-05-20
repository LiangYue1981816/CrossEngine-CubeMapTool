#include "stdafx.h"


#pragma region SH

// https://cseweb.ucsd.edu/~ravir/papers/invlamb/josa.pdf
// http://cseweb.ucsd.edu/~ravir/papers/envmap/envmap.pdf

static const float a[9] = {
	// L0
	0.88622702317327315839103888423359f,

	// L1
	1.0233266579169159691709160221495f,
	1.0233266579169159691709160221495f,
	1.0233266579169159691709160221495f,

	// L2
	0.49541580913239741539330368514174f,
	0.49541580913239741539330368514174f,
	0.49541580913239741539330368514174f,
	0.49541580913239741539330368514174f,
	0.49541580913239741539330368514174f
};

static const float factors[9] = {
	// L0
	0.28209479177387814347403972578039f,

	// L1
	0.48860251190291992158638462283835f,
	0.48860251190291992158638462283835f,
	0.48860251190291992158638462283835f,

	// L2
	0.31539156525252000603089369029571f,
	1.0925484305920790705433857058027f,
	1.0925484305920790705433857058027f,
	0.54627421529603953527169285290134f,
	0.54627421529603953527169285290134f
};

void SHBasis(float basis[], glm::vec3 direction)
{
	// https://zh.wikipedia.org/wiki/%E7%90%83%E8%B0%90%E5%87%BD%E6%95%B0

	float x = direction.x;
	float y = direction.y;
	float z = direction.z;

	basis[0] = factors[0];

	basis[1] = factors[1] * z;
	basis[2] = factors[2] * x;
	basis[3] = factors[3] * y;

	basis[4] = factors[4] * (z * z * 2.0f - x * x - y * y);
	basis[5] = factors[5] * (z * x);
	basis[6] = factors[6] * (y * z);
	basis[7] = factors[7] * (x * x - y * y);
	basis[8] = factors[8] * (x * y);
}

void SH(float sh_red[], float sh_grn[], float sh_blu[], unsigned int color, glm::vec3 direction)
{
	float basis[9] = { 0.0f };

	float r = GET_RED(color) / 255.0f;
	float g = GET_GRN(color) / 255.0f;
	float b = GET_BLU(color) / 255.0f;

	r = pow(r, 1.0f / 2.2f);
	g = pow(g, 1.0f / 2.2f);
	b = pow(b, 1.0f / 2.2f);

	SHBasis(basis, direction);

	for (int index = 0; index < 9; index++) {
		sh_red[index] += basis[index] * r;
		sh_grn[index] += basis[index] * g;
		sh_blu[index] += basis[index] * b;
	}
}

#pragma endregion

#pragma region Sampling

float RadicalInverse(unsigned int bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10f;
}

glm::vec2 Hammersley(unsigned int i, unsigned int n)
{
	return glm::vec2(1.0f * i / n, RadicalInverse(i));
}

glm::vec3 Sampling(glm::vec2 xi)
{
	float phi = 2.0f * PI * xi.x;
	float theta = 2.0f * acos(sqrt(1.0f - xi.y));

	float sinphi = sin(phi);
	float cosphi = cos(phi);
	float sintheta = sin(theta);
	float costheta = cos(theta);

	return glm::vec3(sintheta * cosphi, sintheta * sinphi, costheta);
}

#pragma endregion

#pragma region IrradianceMap

void SaveSH(const char *szFileName, float *sh_red, float *sh_grn, float *sh_blu)
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

void GenerateIrradianceCubeMapSH(CUBEMAP *pCubeMap, float *sh_red, float *sh_grn, float *sh_blu, int samples)
{
	for (int index = 0; index < samples; index++) {
		glm::vec3 direction = glm::normalize(Sampling(Hammersley(index, samples)));
		unsigned int color = CubeMapGetPixelColor(pCubeMap, direction);
		SH(sh_red, sh_grn, sh_blu, color, direction);
	}

	for (int index = 0; index < 9; index++) {
		sh_red[index] *= a[index] * factors[index] * PI * 4.0f / samples;
		sh_grn[index] *= a[index] * factors[index] * PI * 4.0f / samples;
		sh_blu[index] *= a[index] * factors[index] * PI * 4.0f / samples;
	}
}

BOOL GenerateIrradianceCubeMap(CUBEMAP *pCubeMap, CUBEMAP *pIrrMap, int samples)
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
				basis[1] = z;                                                                       \n\
				basis[2] = x;                                                                       \n\
				basis[3] = y;                                                                       \n\
																									\n\
				basis[4] = (z * z * 2.0f - x * x - y * y);                                          \n\
				basis[5] = (z * x);                                                                 \n\
				basis[6] = (y * z);                                                                 \n\
				basis[7] = (x * x - y * y);                                                         \n\
				basis[8] = (x * y);                                                                 \n\
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

	if (CreateVBO(vertices, 4, indices, 6) == FALSE) goto ERR;
	if (CreateFBO(CUBEMAP_WIDTH(pIrrMap), CUBEMAP_HEIGHT(pIrrMap)) == FALSE) goto ERR;
	if (CreateProgram(szShaderVertexCode, szShaderFragmentCode) == FALSE) goto ERR;
	{
		float sh_red[9] = { 0.0f };
		float sh_grn[9] = { 0.0f };
		float sh_blu[9] = { 0.0f };
		GenerateIrradianceCubeMapSH(pCubeMap, sh_red, sh_grn, sh_blu, samples);
		SaveSH("IrradianceSH.output", sh_red, sh_grn, sh_blu);

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

		glViewport(0, 0, CUBEMAP_WIDTH(pIrrMap), CUBEMAP_HEIGHT(pIrrMap));
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
				glReadPixels(0, 0, CUBEMAP_WIDTH(pIrrMap), CUBEMAP_HEIGHT(pIrrMap), GL_BGR, GL_UNSIGNED_BYTE, pIrrMap->faces[index].data);
			}
		}
		glDisableVertexAttribArray(attribLocationPosition);
		glDisableVertexAttribArray(attribLocationTexcoord);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glUseProgram(0);
	}

	goto RET;
ERR:
	rcode = FALSE;
RET:
	DestroyVBO();
	DestroyFBO();
	DestroyProgram();

	return rcode;
}

#pragma endregion

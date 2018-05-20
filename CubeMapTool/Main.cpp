#include "stdafx.h"


void splitfilename(const char *name, char *fname, char *ext)
{
	const char *p = NULL;
	const char *c = NULL;
	const char *base = name;

	for (p = base; *p; p++) {
		if (*p == '/' || *p == '\\') {
			do {
				p++;
			} while (*p == '/' || *p == '\\');

			base = p;
		}
	}

	size_t len = strlen(base);
	for (p = base + len; p != base && *p != '.'; p--);
	if (p == base && *p != '.') p = base + len;

	if (fname) {
		for (c = base; c < p; c++) {
			fname[c - base] = *c;
		}

		fname[c - base] = 0;
	}

	if (ext) {
		for (c = p; c < base + len; c++) {
			ext[c - p] = *c;
		}

		ext[c - p] = 0;
	}
}

#pragma region IrradianceMap

void GenerateIrradianceCubeMap(char szCubeMapFileNames[6][_MAX_PATH], int width, int height)
{
	CUBEMAP cubemap;
	CUBEMAP irradiancemap;
	CubeMapInit(&cubemap);
	CubeMapInit(&irradiancemap);
	{
		CubeMapLoad(&cubemap, szCubeMapFileNames);
		CubeMapAlloc(&irradiancemap, width, height, 24);

		GenerateIrradianceCubeMap(&cubemap, &irradiancemap, 1024);

		gli::texture_cube textureCube(gli::texture::format_type::FORMAT_RGBA8_UNORM_PACK8, gli::texture_cube::extent_type(width, height), 1);
		{
			for (int face = 0; face < 6; face++) {
				for (int y = 0; y < height; y++) {
					for (int x = 0; x < width; x++) {
						COLORREF color = IMAGE_GetPixelColor(&irradiancemap.faces[face], x, y);
						textureCube.store(gli::extent2d(x, y), face, 0, color);
					}
				}
			}
		}
		gli::save_dds(textureCube, "IrradianceMap.dds");

		IMAGE imgPreview;
		IMAGE_ZeroImage(&imgPreview);
		{
			char szDebugFileName[] = "Preview_Irradiance.bmp";
			PreviewMap(&irradiancemap, &imgPreview);
			IMAGE_SaveBmp(szDebugFileName, &imgPreview);
		}
		IMAGE_FreeImage(&imgPreview);
	}
	CubeMapFree(&cubemap);
	CubeMapFree(&irradiancemap);
}

void GenerateIrradianceCubeMap(int argc, char** argv)
{
	int width, height;
	char szCubeMapFileNames[6][_MAX_PATH];

	if (argc != 10) {
		goto ERR;
	}

	strcpy(szCubeMapFileNames[0], argv[2]);
	strcpy(szCubeMapFileNames[1], argv[3]);
	strcpy(szCubeMapFileNames[2], argv[4]);
	strcpy(szCubeMapFileNames[3], argv[5]);
	strcpy(szCubeMapFileNames[4], argv[6]);
	strcpy(szCubeMapFileNames[5], argv[7]);

	width = atoi(argv[8]);
	height = atoi(argv[9]);

	GenerateIrradianceCubeMap(szCubeMapFileNames, width, height);

	goto RET;
ERR:
	printf("Generate irradiance map fail!\n");
	printf("eg: CubeMapTool.exe -irr PositionX.bmp NegativeX.bmp PositionY.bmp NegativeY.bmp PositionZ.bmp NegativeZ.bmp 128 128\n");
RET:
	return;
}

#pragma endregion

#pragma region Mipmap

void GenerateMipmaps(char szEnvMapFileNames[6][_MAX_PATH])
{
	const static int MAX_LEVEL = 16;

	CUBEMAP mipmaps[MAX_LEVEL];
	for (int mipLevel = 0; mipLevel < MAX_LEVEL; mipLevel++) {
		CubeMapInit(&mipmaps[mipLevel]);
	}

	CUBEMAP cubemap;
	CubeMapInit(&cubemap);
	CubeMapLoad(&cubemap, szEnvMapFileNames);
	{
		int mipLevels = 0;
		while ((1 << mipLevels) < max(CUBEMAP_WIDTH(&cubemap), CUBEMAP_HEIGHT(&cubemap))) mipLevels++;

		for (int mipLevel = 0; mipLevel < mipLevels; mipLevel++) {
			int size = 1 << (mipLevels - mipLevel);
			CubeMapAlloc(&mipmaps[mipLevel], size, size, 24);
		}

		GenerateMipmaps(&cubemap, mipmaps, mipLevels, 1024);

		gli::texture_cube textureCube(gli::texture::format_type::FORMAT_RGBA8_UNORM_PACK8, gli::texture_cube::extent_type(1 << mipLevels, 1 << mipLevels));
		{
			for (int mip = 0; mip < mipLevels; mip++) {
				for (int face = 0; face < 6; face++) {
					for (int y = 0; y < (1 << (mipLevels - mip)); y++) {
						for (int x = 0; x < (1 << (mipLevels - mip)); x++) {
							COLORREF color = IMAGE_GetPixelColor(&mipmaps[mip].faces[face], x, y);
							textureCube.store(gli::extent2d(x, y), face, mip, color);
						}
					}
				}
			}
		}
		gli::save_dds(textureCube, "MipMap.dds");

		for (int mipLevel = 0; mipLevel < mipLevels; mipLevel++) {
			IMAGE imgPreview;
			IMAGE_ZeroImage(&imgPreview);
			{
				char szDebugFileName[_MAX_PATH];
				sprintf(szDebugFileName, "Preview_Mipmap%d.bmp", mipLevel);
				PreviewMap(&mipmaps[mipLevel], &imgPreview);
				IMAGE_SaveBmp(szDebugFileName, &imgPreview);
			}
			IMAGE_FreeImage(&imgPreview);
		}
	}
	CubeMapFree(&cubemap);

	for (int mipLevel = 0; mipLevel < MAX_LEVEL; mipLevel++) {
		CubeMapFree(&mipmaps[mipLevel]);
	}
}

void GenerateMipmaps(int argc, char** argv)
{
	char szEnvMapFileNames[6][_MAX_PATH];

	if (argc != 8) {
		goto ERR;
	}

	strcpy(szEnvMapFileNames[0], argv[2]);
	strcpy(szEnvMapFileNames[1], argv[3]);
	strcpy(szEnvMapFileNames[2], argv[4]);
	strcpy(szEnvMapFileNames[3], argv[5]);
	strcpy(szEnvMapFileNames[4], argv[6]);
	strcpy(szEnvMapFileNames[5], argv[7]);

	GenerateMipmaps(szEnvMapFileNames);

	goto RET;
ERR:
	printf("Generate mipmaps fail!\n");
	printf("eg: CubeMapTool.exe -mip PositionX.bmp NegativeX.bmp PositionY.bmp NegativeY.bmp PositionZ.bmp NegativeZ.bmp\n");
RET:
	return;
}

#pragma endregion

#pragma region LUT

void GenerateLUT(char szFileName[_MAX_PATH], int width, int height)
{
	IMAGE image;
	IMAGE_ZeroImage(&image);
	{
		IMAGE_AllocImage(&image, width, height, 24);
		GenerateLUT(&image, 1024);
		IMAGE_SaveBmp(szFileName, &image);
	}
	IMAGE_FreeImage(&image);
}

void GenerateLUT(int argc, char** argv)
{
	int width, height;
	char szLUTFileName[_MAX_PATH];

	if (argc != 5) {
		goto ERR;
	}

	strcpy(szLUTFileName, argv[2]);

	width = atoi(argv[3]);
	height = atoi(argv[4]);

	GenerateLUT(szLUTFileName, width, height);

	goto RET;
ERR:
	printf("Generate lut fail!\n");
	printf("eg: CubeMapTool.exe -lut LUT.bmp 256 256\n");
RET:
	return;
}

#pragma endregion

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

	glutInitWindowSize(0, 0);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("");
	glewInit();

	if (argc > 1) {
		char *opt = argv[1];

		if (stricmp(opt, "-irr") == 0) {
			GenerateIrradianceCubeMap(argc, argv);
			return 0;
		}

		if (stricmp(opt, "-mip") == 0) {
			GenerateMipmaps(argc, argv);
			return 0;
		}

		if (stricmp(opt, "-lut") == 0) {
			GenerateLUT(argc, argv);
			return 0;
		}
	}

	printf("Error: Params not match!\n");
	printf("eg: CubeMapTool.exe -irr PositionX.bmp NegativeX.bmp PositionY.bmp NegativeY.bmp PositionZ.bmp NegativeZ.bmp 128 128\n");
	printf("eg: CubeMapTool.exe -mip PositionX.bmp NegativeX.bmp PositionY.bmp NegativeY.bmp PositionZ.bmp NegativeZ.bmp\n");
	printf("eg: CubeMapTool.exe -lut LUT.bmp 256 256\n");

	return 0;
}

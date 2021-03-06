#include "stdafx.h"


#pragma region IrradianceMap
static void GenerateEnvIrradianceMap(int argc, char** argv)
{
	if (argc != 5) {
		printf("Generate irradiance map fail!\n");
		printf("eg: CubeMapTool.exe -irr-env Env.dds 128 128\n");
		return;
	}

	const char *szEnvMapFileName = argv[2];
	const int width = atoi(argv[3]);
	const int height = atoi(argv[4]);

	gli::texture2d texEnvMap = LoadTexture2D(szEnvMapFileName);
	gli::texture_cube texIrrMap(gli::FORMAT_RGB32_SFLOAT_PACK32, gli::extent2d(width, height));
	{
		GenerateEnvIrradianceMap(texEnvMap, texIrrMap, 1024);
	}
	gli::save_dds(texIrrMap, "IrrEnv.dds");
	gli::save_dds(Preview(texIrrMap), "IrrPreview.dds");

	return;
}

static void GenerateCubeIrradianceMap(int argc, char** argv)
{
	if (argc != 5) {
		printf("Generate irradiance map fail!\n");
		printf("eg: CubeMapTool.exe -irr-cube Cube.dds 128 128\n");
		return;
	}

	const char *szCubeMapFileName = argv[2];
	const int width = atoi(argv[3]);
	const int height = atoi(argv[4]);

	gli::texture_cube texCubeMap = LoadTextureCube(szCubeMapFileName);
	gli::texture_cube texIrrMap(gli::FORMAT_RGB32_SFLOAT_PACK32, gli::extent2d(width, height));
	{
		GenerateCubeIrradianceMap(texCubeMap, texIrrMap, 1024);
	}
	gli::save_dds(texIrrMap, "IrrCube.dds");
	gli::save_dds(Preview(texIrrMap), "IrrPreview.dds");

	return;
}
#pragma endregion

#pragma region Mipmap
static void GenerateEnvMipmaps(int argc, char** argv)
{
	if (argc != 6) {
		printf("Generate mipmaps fail!\n");
		printf("eg: CubeMapTool.exe -mip-env Env.dds EnvMipmap.dds 256 256\n");
		return;
	}

	const char *szEnvFileName = argv[2];
	const char *szEnvMipmapFileName = argv[3];
	const int width = atoi(argv[4]);
	const int height = atoi(argv[5]);

	gli::texture2d texEnvMap = LoadTexture2D(szEnvFileName);
	gli::texture2d texEnvMipmap(gli::FORMAT_RGB32_SFLOAT_PACK32, gli::extent2d(width, height));
	{
		GenerateEnvMipmaps(texEnvMap, texEnvMipmap, 1024);
	}
	gli::save_dds(texEnvMipmap, szEnvMipmapFileName);
}

static void GenerateCubeMipmaps(int argc, char** argv)
{
	if (argc != 6) {
		printf("Generate mipmaps fail!\n");
		printf("eg: CubeMapTool.exe -mip-cube Cube.dds CubeMipmap.dds 256 256\n");
		return;
	}

	const char *szCubeFileName = argv[2];
	const char *szCubeMipmapFileName = argv[3];
	const int width = atoi(argv[4]);
	const int height = atoi(argv[5]);

	gli::texture_cube texCubeMap = LoadTextureCube(szCubeFileName);
	gli::texture_cube texCubeMipmap(gli::FORMAT_RGB32_SFLOAT_PACK32, gli::extent2d(width, height));
	{
		GenerateCubeMipmaps(texCubeMap, texCubeMipmap, 1024);
	}
	gli::save_dds(texCubeMipmap, szCubeMipmapFileName);
}
#pragma endregion

#pragma region Lut
void GenerateLutMap(int argc, char** argv)
{
	if (argc != 5) {
		printf("Generate lut map fail!\n");
		printf("eg: CubeMapTool.exe -lut Lut.dds 128 128\n");
		return;
	}

	const char *szLutMapFileName = argv[2];
	const int width = atoi(argv[3]);
	const int height = atoi(argv[4]);

	gli::texture2d texLutMap(gli::FORMAT_RGBA8_UNORM_PACK8, gli::extent2d(width, height));
	{
		GenerateLutMap(texLutMap, 1024);
	}
	gli::save_dds(texLutMap, szLutMapFileName);

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

		if (stricmp(opt, "-irr-env") == 0) {
			GenerateEnvIrradianceMap(argc, argv);
			return 0;
		}

		if (stricmp(opt, "-irr-cube") == 0) {
			GenerateCubeIrradianceMap(argc, argv);
			return 0;
		}

		if (stricmp(opt, "-mip-env") == 0) {
			GenerateEnvMipmaps(argc, argv);
			return 0;
		}

		if (stricmp(opt, "-mip-cube") == 0) {
			GenerateCubeMipmaps(argc, argv);
			return 0;
		}

		if (stricmp(opt, "-lut") == 0) {
			GenerateLutMap(argc, argv);
			return 0;
		}
	}

	printf("Error: Params not match!\n");
	return 0;
}

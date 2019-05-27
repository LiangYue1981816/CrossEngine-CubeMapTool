#pragma once


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

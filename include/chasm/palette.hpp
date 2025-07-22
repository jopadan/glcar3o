#pragma once

#include <chasm/math.hpp>

namespace chasm
{

template<typename T = u8x3, size_t N = 256>
struct palette : arr<T,N>
{
	palette(u8* src)
	{
		if(src != nullptr)
			for(size_t i = 0; i < N; i++)
				(*this)[i] = { src[i * 3 + 0], src[i * 3 + 1], src[i * 3 + 2] };
	}
	palette(std::filesystem::path& src)
	{
		size_t len = std::filesystem::file_size(src);
		if(std::filesystem::file_exists(src) && len > 0)
		ifstream ifs(src);
		if(ifs.is_open())
		{
			for(size_t i = 0; i < 256; i++)
			{
				ifs >> std::noskipws >> (*this)[i][0];
				ifs >> std::noskipws >> (*this)[i][1];
				ifs >> std::noskipws >> (*this)[i][2];
			}
			ifs.close();
		}
	}
};

template<GLsizei width = 64>
struct palette_image : std::vector<arr<u8, width>>
{
	struct palette*    pal;
	arr<GLsizei, 256> hist;
	u8 bg_index = 0;
	u8 default_bg_index = 0;
	palette_image(u8* buf, size_t len, const struct palette* palette) : pal(palette), std::vector<array<u8,width>>(len / width)
	{
		assert(len == width * this->size());
		for(size_t i = 0; i < this->size(); i++)
			for(size_t j = 0; j < width; j++)
				(*this)[i][j] = buf[i * width + j];
	}
	u8x4 rgba(size_t i, size_t j)
	{
		assert((i < this->size()) && (j < width));
		const u8 c = (*this)[i][j];
		return { pal[c][0], pal[c][1], pal[c][2], c == 4 ? 0 : 255 };
	}
	u8x4 update_hist()
	{
		for(const u8 i : std::make_index_sequence<256>{})
			hist[i] = std::count(this->cbegin, this->cend(), i);
		// Dominant BG color
		auto max_it = std::max_element(hist.cbegin(), hist.cend());
		default_bg_index = (u8)std::distance(hist.cbegin(),max_it);
		return default_bg_index;
	}
	operator texture<width>()
	{
		return texture<64> dst(*this);
	}
}

template<GLsizei width = 64>
struct texture : std::vector<u8x4, width>>
{
	GLuint id;
	texture(const palette_image& src)
	{
		assert(src[0].size() == width);
		this->resize(src.size());
		// Build RGBA skin texture
		for(size_t i = 0; i < src.size(); i++)
			for(size_t j = 0; j < src[i].size(); j++)
				(*this)[i][j] = src.rgba(i,j);
	}
	bind()
	{
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, this->size(), 0, GL_RGBA, GL_UNSIGNED_BYTE, this->data());
	}
	update_filter()
	{
		glBindTexture(GL_TEXTURE_2D, id);
		GLint f = options->useLinear ? GL_LINEAR : GL_NEAREST;
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,f);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,f);
	}
}

};

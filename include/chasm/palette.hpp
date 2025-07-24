#pragma once

#include <chasm/math.hpp>

using namespace std;
using namespace std::filesystem;

namespace chasm
{

template<typename T = struct u8x3, size_t N = 256>
struct palette : arr<T,N>
{
	palette(path src)
	{
		size_t len = 0;
		if(exists(src))
		{
			len = file_size(src);
			if(len > 0)
			{
				ifstream is(src);
				if(is.is_open())
				{
					for(size_t i = 0; i < 256; i++)
					{
						is >> std::noskipws >> (*this)[i][0];
						is >> std::noskipws >> (*this)[i][1];
						is >> std::noskipws >> (*this)[i][2];
					}
					is.close();
				}
				printf("[NFO][VID][PAL] %s\n", src.c_str());
			}
		}
	}
};

struct palette_image : vector<u8>
{
	arr<GLsizei, 256> hist;
	u8 bg_index = 0;
	u8 default_bg_index = 0;
	palette_image(u8* buf, size_t len)
	{
		this->assign(buf, buf + len - 1);
	}
	u8x4 rgba(size_t i, size_t j, palette<u8x3, 256>* pal) const
	{
		assert((i < (this->size() / opt::vid::tex:w)) && (j < opt::vid::tex:w));
		const u8 c = (*this)[i * opt::vid::tex::w + j];
		return u8x4{ (*pal)[c][0], (*pal)[c][1], (*pal)[c][2], c == 4 ? (u8)0 : (u8)255 };
	}
	u8 update_hist()
	{
		for(size_t i = 0; i < 256; i++)
			hist[i] = std::count(this->cbegin(), this->cend(), i);
		// Dominant BG color
		auto max_it = std::max_element(hist.cbegin(), hist.cend());
		default_bg_index = (u8)std::distance(hist.cbegin(),max_it);
		return default_bg_index;
	}
};

struct texture : vector<u8x4>
{
	GLuint id;
	texture(const palette_image& src, palette<u8x3, 256>* pal)
	{
		this->resize(src.size());
		// Build RGBA skin texture
		for(size_t i = 0; i < this->size(); i++)
			(*this)[i] = src.rgba(i / opt::vid::tex::w,i % opt::vid::tex::w, pal);
	}
	void bind()
	{
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, opt::vid::tex::w, this->size() / opt::vid::tex::w, 0, GL_RGBA, GL_UNSIGNED_BYTE, this->data());
	}
	void update_filter()
	{
		glBindTexture(GL_TEXTURE_2D, id);
		GLint f = opt::vid::filter::linear ? GL_LINEAR : GL_NEAREST;
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,f);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,f);
	}
};

namespace opt::vid
{
	palette<u8x3, 256> def_pal("assets/chasmpalette.act");
	palette<u8x3, 256>* pal = &def_pal;
};
};

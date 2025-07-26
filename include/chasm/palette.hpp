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
	palette_image() = default;
	palette_image(u8* buf, size_t len)
	{
		this->assign(buf, buf + len);
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
	texture() = default;
	texture(const palette_image& src, palette<u8x3, 256>* pal)
	{
		this->resize(src.size());
		// Build RGBA skin texture
		for(size_t i = 0; i < this->size(); i++)
		{
			(*this)[i] = (u8x4){ (*pal)[src[i]][0], (*pal)[src[i]][1], (*pal)[src[i]][2],  (src[i] == (u8)4) ? (u8)0 : (u8)255 };
		}
		load();
		opt::vid::tex::id.push_back(id);
	}
	void load()
	{
		glGenTextures(1, &id);
		bind();
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
		glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, opt::vid::tex::w, this->size() / opt::vid::tex::w, 0, GL_RGBA, GL_UNSIGNED_BYTE, this->data());
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		update_filter();
	}
	void bind()
	{
		// Bind texture
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, id);
	}
	void unbind()
	{
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	void update_filter()
	{
		GLint f = opt::vid::filter::linear ? GL_LINEAR : GL_NEAREST;
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,f);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,f);
	}
	void draw()
	{
		f32x2 min = { (float)opt::vid::w-opt::vid::tex::w, (float)opt::vid::h-opt::vid::tex::h };
		f32x2 max = { (float)opt::vid::w, (float)opt::vid::h };
		bind();
		glBegin(GL_QUADS);
		glTexCoord2f(0,1); glVertex2f(min[0],min[1]);
		glTexCoord2f(1,1); glVertex2f(max[0],min[1]);
		glTexCoord2f(1,0); glVertex2f(max[0],max[1]);
		glTexCoord2f(0,0); glVertex2f(min[0],max[1]);
		glEnd();
		unbind();
	}
};

namespace opt::vid
{
	palette<u8x3, 256> def_pal("assets/chasmpalette.act");
	palette<u8x3, 256>* pal = &def_pal;
};
};

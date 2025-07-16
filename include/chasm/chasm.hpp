#pragma once

namespace chasm
{

static const uint16_t OFF_POLY   0x0000
static const uint16_t OFF_VERT   0x3200
static const uint16_t OFF_VCNT   0x4800
static const uint16_t OFF_PCNT   0x4802
static const uint16_t OFF_SKH    0x4804
static const uint16_t OFF_SKIN   0x4806
static const uint16_t SKIN_W     64
static const    float SCALE3O  = 1.0f/2048.0f;

enum class align
{
	none     = 0 << 0,
	scalar   = 1 << 0,
	vector   = 1 << 1,
	matrix   = 1 << 2,
	adaptive = 1 << 3,
};

template<typename t, size_t n, enum align a = align::adaptive, size_t n_pow2 = std::bit_ceil<size_t>(n)>
struct alignas(n == n_pow2 || a == align::vector && a != align::scalar ? n_pow2 : n) array : std::array<t, n>
{
};

template<typename t, size_t n, enum align a = align::adaptive, size_t n_pow2 = std::bit_ceil<size_t>(n)>
struct vertex : array<t, n, a>
{
	t x() requires(n > 0) { return (*this)[0]; }
	t y() requires(n > 1) { return (*this)[1]; }
	t z() requires(n > 2) { return (*this)[2]; }
	t w() requires(n > 3) { return (*this)[3]; }
};

template<typename T = vertex<uint8_t, 3>, size_t N = 256>
struct palette : array<T,N>
{
	palette(uint8_t* src)
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
struct palette_image : std::vector<array<uint8_t, width>>
{
	struct palette& pal;
	array<GLsizei, 256> hist;
	uint8_t bg_index = 0;
	uint8_t default_bg_index = 0;
	palette_image(uint8_t* buf, size_t len, const struct palette& pal) : std::vector<array<uint8_t,width>>(len / width)
	{
		assert(len == width * this->size());
		for(size_t i = 0; i < this->size(); i++)
			for(size_t j = 0; j < width; j++)
				(*this)[i][j] = buf[i * width + j];
	}
	vertex<uint8_t, 4> rgba(size_t i, size_t j)
	{
		assert((i < this->size()) && (j < width));
		const uint8_t c = (*this)[i][j];
		return { pal[c][0], pal[c][1], pal[c][2], c == 4 ? 0 : 255 };
	}
	uint8_t update_hist()
	{
		for(const uint8_t i : std::make_index_sequence<256>{})
			hist[i] = std::count(this->cbegin, this->cend(), i);
		// Dominant BG color
		auto max_it = std::max_element(hist.cbegin(), hist.cend());
		default_bg_index = (uint8_t)std::distance(hist.cbegin(),max_it);
		return default_bg_index;
	}
	operator texture<width>()
	{
		return texture<64> dst(*this);
	}
}

template<GLsizei width = 64>
struct texture : std::vector<array<vertex<uint8_t, 4>, width>>
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

struct face
{
	vertex<uint16_t, 4>               vi;
	array<vertex<uint16_t, 2>, 4>     uv;
	int16_t                         next;
	int16_t                         dist;
	uint8_t                        group;
	uint8_t                        flags;
	int16_t                       uv_off;

	void draw(const vertex<int16_t,3>* frames, size_t f0, size_t f1, float alpha = 0)
	{
		for(int v = 0;v < 3; v++)
		{
			int vi = p.vi[ v ];
			const vertex<int16_t, 3>& pv0 = frames[f0 * hdr->vcount + vi];
			const vertex<int16_t, 3>& pv1 = frames[f1 * hdr->vcount + vi];
			vertex<float, 3>   outv = ((1-alpha) * pv0 + alpha * pv1) * SCALE;

			vertex<float, 2>& outuv = (p.uv[v] + { 0.0f, 4.0f * p.uv_off }) / (vertex<float,2>)vertex<uint16_t,2>({ hdr->tw << 8, hdr->th << 8 });
			glTexCoord2fv(outuv.data());
			glVertex3fv(outv.data());
		}
		// assert range
		if(p.vi[3] < hdr->vcount)
		{
			int ord[3] = {0,2,3};
			for(int v = 0; v <3; v++)
			{
				int vi=p.vi[ord[v]];
				vertex<int16_t, 3>& pv0 = frames[f0 * hdr->vcount + vi];
				vertex<int16_t, 3>& pv1 = frames[f1 * hdr->vcount + vi];
				vertex<float, 3>  outv = ((1-alpha) * pv0 + alpha * pv1) * SCALE;
				vertex<float, 2>& outuv = (p.uv[ord[v]] + { 0.0f, 4.0f * p.uv_off }) / (vertex<float,2>)vertex<uint16_t,2>({ hdr->tw << 8, hdr->th << 8 });
				glTexCoord2fv(outuv.data());
				glVertex3fv(outv.data());
			}
		}
	}
};

enum class fmt
{
	none =  0 << 0,
	c3o  =  1 << 0,
	car  =  1 << 1 + chasm_3o,
};

struct car
{
	struct animap {
		vertex<uint16_t, 20>              model;
		array<vertex<uint16_t, 2>, 6> sub_model;
	} anims;
	struct gsnd {
		array<uint16_t,  3>                  id;
	} gsnd;
	struct sfx {
		vertex<uint16_t, 8>                 len;
		vertex<uint16_t, 8>                 vol;
	} sfx;
};


struct c3o
{
	array<face             ,  400>  faces;
	array<vertex<int16_t,3>,  256>  overt;
	array<vertex<int16_t,3>,  256>  rvert;
	array<vertex<int16_t,3>,  256> shvert;
	array<vertex<int16_t,2>,  256> scvert;
	uint16_t                       vcount;
	uint16_t                       fcount;
	uint16_t                           th;
};

template<bool> struct is_car        : car, c3o {};
template<>     struct is_car<false> :      c3o {};
template<enum fmt T = fmt::c3o> header_fmt : is_car<F == fmt::car> {};

template<enum fmt T = fmt::c3o>
struct header : header_fmt<T>
{
	constexpr static const uint16_t tw    = 64;
	uint16_t                        tdim  = th * tw;
	uint8_t*                        tdata;
	palette_image<tw>               tpal;
	texture<tw>                     trgba;
};

static float animationTime = 0.0f, frameDuration = 0.1f;
static int animating = 0;

typedef struct { size_t start, count; } AnimInfo;

template<enum fmt T = fmt::c3o>
struct model : std::vector<uint8_t>
{
	union {
		header<T>*        hdr;
		header<fmt::car>* car;
		header<fmt::c3o>* c3o;
	};
	vertex<float,3>           pos;
	struct anim
	{
		size_t beg;
		size_t cnt;
	};
	struct frame
	{
		size_t beg;
		size_t cnt;
	};
	struct anims : std::vector<anim>
	{
		size_t idx; // currentAnim 
		size_t len; // time
	}
	struct frames : std::vector<frame>
	{
		size_t idx; // animFrameIndex
		size_t len; // duration
	}
	size_t sounds_offset()
	{
		size_t dst = hdr->th * hdr->tw + sizeof(header_fmt<fmt::car>) + OFF_SKIN;
		ds-t += std::accumulate(anims.model.cbegin(), anims.model.cend(), (uint16_t)0);
		for(size_t i = 0; i < anims.sub_model.size(); i++)
		{
			const size_t sum = anims.sub_model[i].sum();
			dst += sum == 0 ? 0 : sum + OFF_SKIN;
		}
		return dst;
	}
	bool verify_length(size_t len)
	{
		size_t acc == sounds_offset();
		for(size_t i = 0; i < sfx.len.size(); i++)
			acc += sfx.len[i];
		return acc == len;
	}
	model(uint8_t* buf, size_t len, const struct palette& palette) : pal(palette)
	{
		this->resize(len);
		for(size_t i = 0; i < len; i++)
			(*this)[i] = buf[i];
	}
	model(std::filesystem::path fn, const struct palette& palette) : pal(palette)
	{
		len = std::filesystem::file_size(n);
		if(std::filesystem::file_exists(fn) && len > 0)
		{
			this->resize(len);
			ifstream is(fn);
			if(is.is_open())
			{
				for(size_t i = 0; i < len; i++)
					is >> std::noskipws >> (*this)[i];
				hdr = this->data();
				is.close();
			}
		}
	}
	load()
	{
		// Compute center
		update_center();
		// Load palette image skin and convert to rgba texture
		pal_skin = palette_image(this->cbegin() + sizeof(struct header_fmt<T>), hdr->th * hdr->tw, pal);
		skin     = (texture<64>)pal_skin;
		skin.bind();
		skin.update_filter();

		if(T == fmt::car)
		{
			auto frames = this->cbegin() + sizeof(struct header_fmt<car>) + hdr->th * hdr->tw;
			size_t frame_count = std::distance(this->cbegin(), frames) / hdr->vcount * sizeof(vertex<int16_t, 3>);
			vertex<int16_t, 3>& frames = (vertex<int16_t, 3>*)frames;

			size_t off=0;
			for(uint16_t& anim : hdr->anims.model)
			{
				if(anim)
				{
					const size_t n = anim / (hdr->vcount * sizeof(vertex<int16_t,3>));
					anims.push_back(anim_info{off, n});
					off+=n;
				}
			}
			if(anims.empty())
				anims.push_back({0, frame_count});
			anims.idx  = 0;
			frames.idx = 0;
		}
	};
	vertex<float,3>& update_center()
	{
		vertex<int16_t,3> *vv = (vertex<int16_t,3>*)overt;
		vertex<int16_t,3> min{INT16_MAX};
		vertex<int16_t,3> max{INT16_MIN};
		for(int i = 0; i < vcount; i++)
		{
			min = std::min(min, vv[i]);
			max = std::max(max, vv[i]);
		}
		center = (min + max) * 0.5f;
		return center;
	}
	void apply_transform()
	{
		glTranslatef(center.x(), center.y(), center.z());
		glRotatef(options.rot.y(), 0.0f,1.0f,0.0f);
		glRotatef(options.rot.x(), 1.0f,0.0f,0.0f);
		glTranslatef(-center.x(), -center.y(), -center.z());
	}
	void display(void)
	{
		glClearColor(bgColor[0],bgColor[1],bgColor[2],1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// set color to white
		glColor3fv(color::white.data());

		// reset modelview transform
		glLoadIdentity();
		// apply global options modelview transform
		glTranslatef(options.pos.x(), options.pos.y(), -5.0f / options.zoom);

		if(options.draw_perspective)
		{
			apply_transform();

			const size_t beg = anims.data[anims.cur].start;
			const size_t cnt = anims.data[anims.cur].count;

			glBindTexture(GL_TEXTURE_2D, texture.id);

			glBegin(GL_TRIANGLES);
				for(uint16_t i = 0; i < hdr->fcount; i++)
					hdr->faces[i].draw(frames,
					                   beg + anims.frame_idx,
					                   beg + ((anims.frame_idx + 1) % cnt),
							   options.animating ? cnt : 0);
			glEnd();
		}
		if(options.draw_ortho2d)
		{
			draw_overlay();
			draw_modelinfo();
		}

		glutSwapBuffers();
	}
};


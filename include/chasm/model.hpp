#pragma once

#include <chasm/model.hpp>

namespace chasm
{

struct model : std::vector<uint8_t>
{
	static constexpr const f32 SCALE3O = 1.0f/2048.0f;
	static constexpr enum fmt type(uint8_t* buf, size_t len)
	{
		if(buf != nullptr && len > 0)
		{
			if(((struct c3o*)buf)->fmt(len))
				return fmt::c3o;
			if(((struct car*)buf)->fmt(len))
				return fmt::car;
		}
		return fmt::none;
	}
	enum format fmt;
	struct pos
	{
		size_t c3o;
		size_t tex;
		size_t ani;
		size_t snd;
	} pos;
	struct ani
	{
		std::filesystem::path loc;
		size_t                len;
		size_t                off;
	};
	std::vector<struct ani> ani;
	size_t load_ani(std::filesystem::path ani_path)
	{
		const size_t len = std::filesystem::file_size(ani_path);
		if((ani.size() > 0 && ani[ani.back()].len > 0) || ani.empty())
			ani.resize(ani.size() + 1);
		ani[ani.back()] = { ani_path, len, ani.size() > 1 ? ani[ani.back() - 1].off + ani[ani.back() - 1].len : 0 };

		if(std::filesystem::exists(ani_path))
		{
			std::ifstream is(ani_path);
			if(is.is_open())
			{
				/* verify if first u16 matches vertex count */
				uint16_t cnt;
				is >> std::noskipws >> cnt;
				cnt = le16toh(cnt);
				if(cnt == c3o()->cnt.vtx)
					ani[ani.back()].len -= 2;
				else
					is.seekg(0, std::ios::beg);

				/* alloc space for animation vertices */
				this->resize(pos.ani + ani[ani.back()].off + ani[ani.back()].len);

				/* read animation bytes */
				for(size_t i = this.size() - ani[ani.back()].len; i < (*this).size(); i++)
					is >> std::noskipws >> (*this)[i];
				is.close();
			}
		}
		return this->size() - ani[ani.back()].len;
	}

	model(std::filesystem::path src)
	{
		if(!std::filesystem::file_exists(src)) return;
		(*this).resize(std::filesystem::file_size(src));
		if((*this).size() > 0)
		{
			std::ifstream is(src);
			if(is.is_open())
			{
				for(size_t i = 0; i < (*this).size(); i++)
					is >> std::noskipws >> (*this)[i];
				is.close();
			}
		}
		fmt = type(this->data(), this->size());
		switch(fmt)
		{
			case fmt::c3o:
				pos.c3o = 0;
				pos.tex = sizeof(struct c3o);
				pos.ani = pos.tex + ((struct c3o*)this->data())->tex.h * ((struct c3o*)this->data())->tex.w;
				pos.snd = load_ani(src.replace_extension(".ani"));
				break;
			case fmt::car:
				pos.c3o = sizeof(struct car);
				pos.tex = sizeof(struct car) + sizeof(struct c3o);
				pos.ani = pos.tex + ((struct car*)this->data())->tex.h;
				pos.snd = this->size() - ((struct car*)buf)->sfx.size();
				break;
			case fmt::none:
			default:
		}
	}

	struct car& car() { return reinterpret_cast<struct car&>(*(*this)[0      ]); }
	struct c3o& c3o() { return reinterpret_cast<struct c3o&>(*(*this)[pos.c3o]); }
	struct tex& tex() { return reinterpret_cast<struct tex&>(*(*this)[pos.tex]); }
	vertex<i16,3>* ani(size_t i = 0) { return reinterpret_cast<vertex<i16,3>*>(*this)[pos.ani]; }
	u8* snd(size_t i = 0) { return reinterpret_cast<struct snd&>(*(*this)[pos.snd]); }
};
};
/*
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
*/

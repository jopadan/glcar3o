#pragma once

#include <chasm/format.hpp>

namespace chasm
{

struct snd_info
{
	std::vector<u8> buf;
	path  name;
	snd_info(path src)
	{
		if(exists(src))
		{
			const size_t len = file_size(src);
			if(len > 0)
			{
				name = src;
				buf.resize(len);
				ifstream is(src);
				if(is.is_open())
				{
					for(size_t i = 0; i < len; i++)
						is >> std::noskipws >> buf[i];
					is.close();
				}
			}
		}
	}
};

struct model : std::vector<uint8_t>
{
	static constexpr const f32 SCALE3O = 1.0f/2048.0f;

	struct fmt_stat
	{
		enum fmt    fmt;
		struct c3o* c3o;
		u16         cnt;
		u16          th;
		struct      len
		{
			size_t ani;
			size_t sfx;
			size_t total;
		} len;
	};

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

	static constexpr fmt_stat stat(uint8_t* buf, size_t len)
	{
		fmt_stat sb;
		sb.len.total = len;
		switch(type(buf, len))
		{
			case fmt::c3o:
				sb.fmt     = fmt::c3o; sb.c3o = (struct c3o*)buf; sb.cnt = sb.c3o->cnt.vtx;
				sb.th      = sb.c3o->tex.h;
				sb.len     = { 0, 0 };
				return sb;
			case fmt::car:
				sb.fmt     = fmt::car; sb.c3o = (struct c3o*)(buf + sizeof(struct car));
				sb.cnt     = sb.c3o->cnt.vtx; sb.th = sb.c3o->tex.h / opt::vid::tex::w;
				sb.len     = { .ani = (u16)((struct car*)buf)->ani.size(), .sfx = (u16)((struct car*)buf)->sfx.size() };
				return sb;
			default: break;
		}
		return {};
	}

	static constexpr fmt_stat stat(path& src)
	{
		fmt_stat sb = {};
		if(exists(src))
		{
			sb.len.total = file_size(src);
			if(sb.len.total > 0)
			{
				ifstream ifs(src);
				if(ifs.is_open())
				{
					std::vector<uint8_t> buf(sb.len.total);
					for(size_t i = 0; i < sb.len.total; i++)
						ifs >> std::noskipws >> buf[i];
					ifs.close();
					return stat(buf.data(), buf.size());
				}
			}
		}
		return sb;
	}
	enum fmt fmt;
	struct pos
	{
		size_t c3o;
		size_t ani;
		size_t snd;
	} pos;

	std::vector<ani> anis;
	palette_image    skin;
	texture          skin_rgba;
	model(path src, vector<path> anim_files = {})
	{
		if(!exists(src)) return;
		(*this).resize(file_size(src));
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
			{
				struct c3o* c3o = ((struct c3o*)this->data());
				skin = palette_image(c3o->tex.data(), c3o->tex.h * c3o->tex.w);
				skin_rgba = texture(skin, opt::vid::pal);
				if(anim_files.size() > 0)
				{
					for(size_t i = 0; i < anim_files.size(); i++)
						if(exists(anim_files[i]))
							anis.push_back(ani(anim_files[i], ((struct c3o*)this->data())->cnt.vtx));
				}
				break;
			}
			case fmt::car:
			{
				struct c3o* c3o = ((struct car*)this->data())->c3o();
				skin = palette_image(c3o->tex.data(), c3o->tex.h);
				skin_rgba = texture(skin, opt::vid::pal);
				pos.snd = this->size() - ((struct car*)this->data())->sfx.size();
				break;
			}
			case fmt::none:
			default:
				break;
		}
	}
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

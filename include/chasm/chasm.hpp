#pragma once

#include <chasm/model.hpp>


/*
size_t csm_model_car_anim_count(model* hdr)
{
	hdr->frame_count = csm_model_car_frame_count(hdr->car);
	size_t off = 0;
	for(size_t i = 0; i < 20; i++)
	{
		uint16_t b = hdr->car->anims.model[i];
		if(b)
		{
			size_t n = b / (hdr->car->vcount * sizeof(i16x3));
			hdr->anims[hdr->anim_count].start = off;
			hdr->anims[hdr->anim_count].count = n;
			off += n; hdr->anim_count++;
		}
	}
	if(hdr->anim_count == 0)
	{
		hdr->anims[0].start = 0;
		hdr->anims[0].count = hdr->frame_count;
		hdr->anim_count = 1;
	}
	hdr->anim_current   = 0;
	hdr->anim_frame_idx = 0;
	printf("[NFO][MDL] anim_count: %zu frame_count: %zu\n", hdr->anim_count, hdr->frame_count);
	return hdr->anim_count;
}
file.c3o()[

    // build WAV buffers and apply volume factor
    uint32_t totalBytes=0; for(int b=0;b<8;b++) totalBytes+=hdr->sfx.len[b];
    long audio_off=rawSize-totalBytes, pos=audio_off;
    for(int b=0;b<8;b++){
        uint16_t len=hdr->sfx.len[b];
        if(len){
            uint32_t ws=44+len;
            uint8_t *buf=malloc(ws);
            memcpy(buf+0,"RIFF",4);
            uint32_t chsz=36+len; memcpy(buf+4,&chsz,4);
            memcpy(buf+8,"WAVEfmt ",8);
            uint32_t sub1=16; memcpy(buf+16,&sub1,4);
            uint16_t pcm=1,ch=1; memcpy(buf+20,&pcm,2); memcpy(buf+22,&ch,2);
            uint32_t rate=11025; memcpy(buf+24,&rate,4);
            uint32_t brate=rate*ch; memcpy(buf+28,&brate,4);
            uint16_t align=ch;   memcpy(buf+32,&align,2);
            uint16_t bps=8;      memcpy(buf+34,&bps,2);
            memcpy(buf+36,"data",4);
            uint32_t dlen=len;   memcpy(buf+40,&dlen,4);
            for(int i=0;i<len;i++){
                uint8_t s = rawData[pos+i];
                float centered = (float)s - 128.0f;
                centered *= VOLUME_FACTOR;
                int ns = (int)(centered + 128.0f);
                if(ns<0) ns=0; else if(ns>255) ns=255;
                buf[44+i] = (uint8_t)ns;
            }
            wavBuffers[b]=buf;
            wavBufferLens[b]=ws;
        }
        pos+=len;
    }

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
*/

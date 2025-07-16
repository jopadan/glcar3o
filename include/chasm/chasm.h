#pragma once

#ifdef __cplusplus
extern "C" {
#include <cstdint>
#include <cstddef>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#else
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#endif
#include <sys/stat.h>
#include <GL/gl.h>
#include <GL/glcorearb.h>

typedef uint32_t u32;
typedef  int32_t i32;
typedef uint16_t u16;
typedef  int16_t i16;
typedef  uint8_t  u8;
typedef   int8_t  i8;
typedef    float f32;

#pragma pack(push,1)
typedef struct {
	u16    vi[4];
	u16 uv[4][2];
	u16     next;
	u16  distant;
	u8     group;
	u8     flags;
	u16   uv_off;
} face;

typedef struct {
union
{
	struct { i16 x,y,z; };
	i16 xyz[3];
};
} i16x3;

typedef struct {
union
{
	struct { i16 x,y; };
	i16 xy[2];
};
} i16x2;

typedef struct {
union
{
	struct { u8 r, g, b; };
	u8 rgb[3];
};
} u8x3;

typedef struct {
union
{
	struct { u8 r, g, b, a; };
	u8 rgba[3];
};
} u8x4;

typedef u8x3 palette[256];

typedef struct {
	struct animap {
		u16 model[20];
		u16 sub_model[6][2];
	} anims;
	struct gsnd {
		u16 id[3];
	} gsnd;
	struct sfx {
		u16 len[8];
		u16 vol[8];
	} sfx;
	face   faces[400];
	i16x3  overt[256];
	i16x3  rvert[256];
	i16x3 shvert[256];
	i16x2 scvert[256];
	u16        vcount;
	u16        fcount;
	u16            th;
} car_header;

typedef struct
{
	face   faces[400];
	i16x3  overt[256];
	i16x3  rvert[256];
	i16x3 shvert[256];
	i16x2 scvert[256];
	u16        vcount;
	u16        fcount;
	u16            th;
} c3o_header;

enum format
{
	CHASM_FORMAT_NONE = 0 << 0,
	CHASM_FORMAT_3O   = 1 << 0, 
	CHASM_FORMAT_CAR  = CHASM_FORMAT_3O + 1 << 1,
};

typedef struct anim_info
{
	size_t start;
	size_t count;
} anim_info;

typedef struct model
{
	/* raw file data */
	u8*              data;
	/* pointer to header */
	c3o_header*       c3o;
	car_header*       car;
	/* pointer to post header suffix tail */
	u8*             tdata;
	i16x3*    anim_frames;
	/* content description */
	size_t    frame_count;
	enum format       fmt;
	size_t            len;
	char         name[32];
	GLsizei            th;
	GLsizei            tw;
	GLsizei          tdim;
	palette*          pal;
	u8x4*           trgba;
	anim_info   anims[20];
	size_t     anim_count;
	size_t   anim_current;
	size_t anim_frame_idx;
} model;

typedef struct config
{
	bool draw_ortho2d;
	bool draw_perspective;
	palette* pal;
} config;

config settings;
#pragma pack(pop)

size_t acc(u16* src, size_t cnt, size_t init)
{
	for(size_t i = 0; i < cnt; i++)
		init += src[i];
	return init;
}

u8x4* tpal2rgba(u8* buf, size_t len, palette* pal)
{
	if(buf == NULL || pal == NULL || len <= 0) return NULL;

	u8x4* dst = (u8x4*)calloc(sizeof(u8x4), len);
	for(size_t i = 0; i < len; i++)
	{
		const size_t
		idx      =     buf[i];
		dst[i].r =  (*pal)[idx].r;
		dst[i].g =  (*pal)[idx].g;
		dst[i].b =  (*pal)[idx].b;
		dst[i].a = ((*pal)[idx].r == 4 && (*pal)[idx].g == 4 && (*pal)[idx].b == 4) ? 0 : 255;
	}
	return dst;
}

palette* csm_palette_create_fn(const char* filename)
{
	struct stat sb;
	palette* dst;

	/* check if file of sufficient length exists */
	if(filename == NULL || stat(filename, &sb) != 0 || sb.st_size < 768) return NULL;
	printf("[NFO][PAL] %s\n", filename);

	/* allocate memory for model data */
	dst = (palette*)calloc(sizeof(palette), 1);

	/* read file contents */
	FILE* fp = fopen(filename, "rb");
	long err = fread(dst, sizeof(palette), 1, fp);
	fclose(fp);
	if (err != 1) { free(dst); return NULL; }

	return dst;
}

palette* csm_palette_delete(palette* pal)
{
	if(pal != NULL)
	{
		free(pal);
		pal = NULL;
	}
	return pal;
}

size_t csm_model_car_frame_count(car_header* hdr)
{
	size_t dst = acc(hdr->anims.model, 20, 0);

	for(size_t i = 0; i < 6; i++)
	{
		const size_t sum = acc(hdr->anims.sub_model[i], 2, 0);
		dst += sum == 0 ? 0 : dst + sizeof(c3o_header);
	}
	return dst;
}

size_t csm_model_car_sfx_len(car_header* hdr)
{
	return acc(hdr->sfx.len, 8, 0);
}

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

void csm_model_format_print(enum format fmt)
{
	switch(fmt)
	{
		case CHASM_FORMAT_CAR : printf("[NFO][FMT] .CAR - Chasm: The Rift CARacter animation model\n"); break;
		case CHASM_FORMAT_3O  : printf("[NFO][FMT] .3O  - Chasm: The Rift 3O model\n"); break;
		case CHASM_FORMAT_NONE: printf("[NFO][FMT] Unknown model format!\n"); break;
	}
}


enum format csm_model_format(const u8* buf, size_t len)
{
	size_t       hdr_len = sizeof(c3o_header);
	const size_t car_len = sizeof(car_header) - hdr_len;
	size_t            tw = 64;
	c3o_header*      c3o = (c3o_header*)buf;
	car_header*      car = (car_header*)buf;

	if(buf != NULL && len > 0)
	{
		printf("%zu + %zu / %zu\n", hdr_len, (size_t)c3o->th, len);
		if(hdr_len + c3o->th * tw == len)
			return CHASM_FORMAT_3O;

		hdr_len += car_len;
		tw       = csm_model_car_frame_count(car);
		tw      += csm_model_car_sfx_len(car);

		printf("%zu + %zu\n", hdr_len, (size_t)car->th);
		if(hdr_len + car->th + tw == len)
			return CHASM_FORMAT_CAR;
	}
	return CHASM_FORMAT_NONE;
}

model* csm_model_reset(model* dst)
{
	if(dst != NULL)
	{

		if(dst->data != NULL)
		{
			free(dst->data);
			dst->data = NULL;
		}
		memset(dst, sizeof(model), 1);
		dst->tw             = 64;

		if(dst->trgba)
		{
			free(dst->trgba);
			dst->trgba = NULL;
		}
		dst->anim_frames = NULL;
	}
	return dst;
}

model csm_model_create(u8* buf, size_t len)
{
	model dst = { .data = buf, .len = len };
	if(dst.data == NULL && dst.len <= 0) return dst;

	dst.data = buf;
	dst.len  = len;

	/* identify format */
	dst.fmt = csm_model_format(dst.data, dst.len);

	switch(dst.fmt)
	{ 
		case CHASM_FORMAT_3O:
		{
			dst.car         = (car_header*)NULL;
			dst.c3o         = (c3o_header*)dst.data;
			dst.tw          = 64;
			dst.th          = dst.c3o->th;
			dst.tdim        = dst.th * dst.tw;
			dst.tdata       = dst.data + sizeof(c3o_header);
			dst.pal         = settings.pal;
			dst.trgba       = tpal2rgba(dst.tdata, dst.tdim, dst.pal);
			dst.anim_frames = (i16x3*)(dst.tdata + dst.tdim);
			break;
		}
		case CHASM_FORMAT_CAR:
		{
			dst.car         = (car_header*)dst.data;
			dst.c3o         = (c3o_header*)dst.data + sizeof(car_header) - sizeof(c3o_header);
			dst.tw          = 64;
			dst.th          = dst.car->th / dst.tw;
			dst.tdim        = dst.th * dst.tw;
			dst.tdata       = dst.data + sizeof(car_header);
			dst.pal         = settings.pal;
			dst.trgba       = tpal2rgba(dst.tdata, dst.tdim, dst.pal);
			dst.anim_frames = (i16x3*)(dst.tdata + dst.tdim);
			dst.anim_count  = csm_model_car_anim_count(&dst);
			break;
		}
		case CHASM_FORMAT_NONE:
		default:
			csm_model_reset(&dst); break;
	}
	return dst;
}

model csm_model_create_fn(const char* filename)
{
	struct stat sb;
	model dst = {0};

	/* check if file of sufficient length exists */
	if(filename == NULL || stat(filename, &sb) != 0 || sb.st_size <= 0) return dst;

	/* allocate memory for model data */
	dst.len    = sb.st_size;
	dst.data   = (u8*)calloc(dst.len, 1);
	/* read file contents */
	FILE* fp = fopen(filename, "rb");
	long err = fread(dst.data, dst.len, 1, fp);
	fclose(fp);
	if (err != 1) { csm_model_reset(&dst); return dst; }

	/* parse model header */
	return csm_model_create(dst.data, dst.len);
}

model* csm_model_delete(model* ptr)
{
	if(ptr != NULL)
	{
		csm_model_reset(ptr);
		free(ptr);
		ptr = NULL;
	}
	return ptr;
}

#ifdef __cplusplus
};
#endif

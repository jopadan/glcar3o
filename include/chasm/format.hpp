#pragma once

#include <chasm/math.hpp>
#include <chasm/palette.hpp>

namespace chasm
{

enum class fmt
{
	none =  0 << 0,
	c3o  =  1 << 0,
	car  = (1 << 1) + c3o
};

#pragma pack(push,1)
struct face
{
	struct                  { u16x4       vi; arr<u16x2, 4>   uv;                           };
	struct link             { i16       next; i16           dist;                           } link;
	struct conf             { u8       group; u8           flags;                           } conf;
	struct                  { u16     uv_off;                                               };
};

struct c3o
{
	struct                  { arr<face, 400> pol;                                           };
	struct vtx              { arr<i16x3, 256>  o;  arr<i16x3, 256>   r;
	                          arr<i16x3, 256> sh;  arr<i16x2, 256>  sc;                     } vtx;
	struct cnt              { u16            vtx; u16              pol;                     } cnt;
	struct tex              { u16 h; static constexpr const u16 w = opt::vid::tex::w; 
	u8* data()              { return ((u8*)this + sizeof(struct tex)); }                    } tex;
        bool   fmt (size_t len) { return sizeof(struct c3o) + tex.h * opt::vid::tex::w == len; }
};

struct car
{
	struct AniMap           { arr<u16, 20> mdl; arr<u16x2,6> sub;
	size_t size()           { size_t dst = acc(mdl.cbegin(), mdl.cend(), (size_t)0);
	                          for(size_t i = 0; i < sub.size(); i++)
	                                if(sub[i][0] + sub[i][1] > 0)
	                                     dst += dst + sizeof(struct c3o);
	                          return dst;                                                  }} ani;
	struct GSND             { u16x3 id;                                                     } snd;
	struct SFX              { u16x8 len; u16x8 vol;
	u8* data()              { struct car* ptr = (struct car*)((u8*)this) - (sizeof(struct car) - sizeof(this)); return (u8*)ptr + sizeof(struct car) + ptr->c3o()->tex.h + ptr->ani.size() + ptr->sfx.size(); }
	size_t size()           { return acc(&len[0], &len[0] + 8, 0); }                        } sfx;
        struct c3o*     c3o()   { return ((struct c3o*)((u8*)this + sizeof(struct car)));       }
	bool   fmt(size_t len)  { return sizeof(struct c3o) + sizeof(struct car) + c3o()->tex.h 
	                          + this->ani.size() + this->sfx.size() == len;                 }
};
#pragma pack(pop)

struct ani : vector<i16x3>
{
	path name;
	static constexpr const size_t off(uint8_t* buf, size_t vcount)
	{
		if(buf != nullptr && le16toh(*(u16*)buf) == vcount)
			return 2;
		return 0;
	}
	static constexpr const size_t len(uint8_t* buf, size_t len, size_t vcount)
	{
		if(buf != nullptr && len > 2)
			len -= off(buf, vcount);
		return len;
	}
	static constexpr const size_t cnt(uint8_t* buf, size_t buf_len, size_t vcount)
	{
		return len(buf, buf_len, vcount) / sizeof(i16x3);
	}
	static constexpr const size_t off(path src, size_t vcount)
	{
		size_t len = 0;
		if(exists(src))
		{
			len = file_size(src);
			if(len > 2)
			{
				ifstream is(src);
				if(is.is_open())
				{
					/* verify if first u16 matches vertex count N */
					uint16_t cnt;
					is >> std::noskipws >> cnt;
					is.close();
					return off((u8*)&cnt, vcount);
				}
			}
		}
		return 0;
	}
	static constexpr const size_t len(path src, size_t vcount)
	{
		size_t len = 0;
		if(exists(src))
		{
			len = file_size(src);
			if(len > 2)
			{
				ifstream is(src);
				if(is.is_open())
				{
					/* verify if first u16 matches vertex count N */
					uint16_t cnt;
					is >> std::noskipws >> cnt;
					is.close();
					if(le16toh(cnt) == vcount)
						len -= 2;
					return len;
				}
			}
		}
		return 0;
	}
	static constexpr const size_t cnt(path src, size_t vcount)
	{
		return len(src, vcount) / sizeof(i16x3);
	}

	ani(path src, size_t vcount) : name(src)
	{
		size_t len = 0;
		if(exists(src))
		{
			size_t pos = off(src, vcount);
			len = file_size(src) - pos;
			if(len > 0)
			{
				ifstream is(src);
				if(is.is_open())
				{
					this->resize(len / sizeof(i16x3));
					is.seekg(pos, ios::beg);
					for(size_t i = 0; i < len / sizeof(i16x3); i++)
					{
						is >> std::noskipws >> (*this)[i][0];
						is >> std::noskipws >> (*this)[i][1];
						is >> std::noskipws >> (*this)[i][2];
					}
					is.close();
				}
			}
		}
	}
};

};

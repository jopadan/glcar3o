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

#pragma push(pack,1)
struct face
{
	struct                  { u16x4       vi; arr<u16x2, 4>   uv;                           };
	struct link             { i16       next; i16           dist;                           } link;
	struct conf             { u8       group; u8           flags;                           } conf;
	struct                  { u16     uv_off;                                               };
};

struct c3o
{
	struct                  { arr<struct face, 400> pol;                                    };
	struct vtx              { arr<i16x3, 256>         o; arr<i16x3, 256>   r;
	                          arr<i16x3, 256>        sh; arr<i16x2, 256>  sc;               } vtx;
	struct cnt              { u16                   vtx; u16             pol;               } cnt;
	struct tex              { u16 h; static constexpr const u16 w = 64;                     } tex;
	bool   fmt (size_t len) { return sizeof(struct c3o) + this->tex.h * this->tex.w == len; }
};

struct car
{
	struct AniMap           { arr<u16, 20> mdl; arr<u16x2,6> sub;
	size_t size()           { size_t dst = acc(mdl.cbegin(), mdl.cend(), (size_t)0);
	                          for(size_t i = 0; i < sub.size(); i++)
	                                if(acc(sub[i].cbegin(), sub[i].cend(), (size_t)0) > 0)
	                                     dst += dst + sizeof(struct c3o);
			          return dst;                                                  }} ani;

	struct GSND             { u16x3 id;                                                     } gsnd;
	struct SFX              { u16x8 len; u16x8 vol;                                         }
	size_t size()           { return acc(len.cbegin(), len.cend(), 0); }                    } sfx;
        struct c3o*     c3o()   { return ((struct c3o*)((u8*)this + sizeof(struct car)));       }
	bool   fmt(size_t len)  { return sizeof(struct c3o) + sizeof(struct car) + c3o()->tex.h 
	                          + this->ani.size() + this->sfx.size() == len;                 }
};
#pragma pop(pack,1)

};

#pragma once

#include <chasm/math.hpp>

namespace chasm
{
	namespace opt
	{
		namespace vid
		{
			namespace draw
			{
				namespace ortho2d
				{
					bool time;
					bool fps;
					bool info;
					bool texture;
					bool menu;
				};
				namespace perspective
				{
				};
			};
			namespace tex
			{
				constexpr u16 w = 64;
			};
			namespace filter
			{
				bool linear;
			};
		};
	};
};

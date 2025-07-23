#pragma once

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
				constexpr uint16_t w = 64;
			};
			namespace filter
			{
				bool linear;
			};
		};
	};
};

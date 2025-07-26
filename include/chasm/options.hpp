#pragma once

#include <chasm/math.hpp>

namespace chasm
{
	namespace opt
	{
		namespace vid
		{
			GLsizei w = 640;
			GLsizei h = 480;
			namespace draw
			{
				bool shading = true;
				bool culling = true;
				bool wireframe = false;
				namespace camera
				{
					float zoom = 1.0f;
					float panx = 0.0f;
					float pany = 0.05f;
					float anglex = 0.0f;
					float angley = 0.0f;
					constexpr inline void ortho2d_beg()
					{
						glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
						gluOrtho2D(0,opt::vid::w,0,opt::vid::h);
						glMatrixMode(GL_MODELVIEW);  glPushMatrix(); glLoadIdentity();
					}
					constexpr inline void ortho2d_end()
					{
						glMatrixMode(GL_PROJECTION); glPopMatrix();
						glMatrixMode(GL_MODELVIEW);  glPopMatrix();
					}
				};
				namespace ortho2d
				{
					bool time;
					bool fps;
					bool info;
					bool texture = true;
					bool menu;
				};
				namespace perspective
				{
				};
			};
			namespace tex
			{
				constexpr u16 w = 64;
				u16 h           =  1;
				std::vector<GLuint> id;
			};
			namespace filter
			{
				bool linear;
			};
		};
	};
};

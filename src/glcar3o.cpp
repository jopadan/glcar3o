#include <chasm/model.hpp>
#include <GL/freeglut.h>

using namespace chasm;


std::vector<model> models;

void display()
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glCullFace(GL_FRONT);
	glFrontFace(GL_CCW);
	// Bind texture
	glBindTexture(GL_TEXTURE_2D, 0);
	// Texture preview
	if(opt::vid::draw::ortho2d::texture){
		opt::vid::draw::camera::ortho2d_beg();
		models.back().skin_rgba.draw();		
		opt::vid::draw::camera::ortho2d_end();
	}
	glutSwapBuffers();
}

static void reshape(int w,int h){
	opt::vid::w = w; opt::vid::h = h;
	glViewport(0,0,w,h);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
}

static void keyboard(unsigned char k,int x,int y){
	(void)x;(void)y;
	switch(k){
		case 27: exit(0);
	}
}
std::vector<uint8_t> read_file(path ifn)
{
	std::vector<uint8_t> buf;
	if(exists(ifn))
	{
		size_t len = file_size(ifn);
		if(len > 0)
		{
			ifstream ifs(ifn);
			if(ifs.is_open())
			{
				buf.resize(len);
				for(size_t i = 0; i < len; i++)
					ifs >> std::noskipws >> buf[i];
				ifs.close();
			}
		}
	}
	return buf;
}
constexpr inline const char* fmt_c_str(enum fmt fmt)
{
	switch(fmt)
	{
		case fmt::c3o: return "3O";
		case fmt::car: return "CAR";
		case fmt::none:
		default: return "UNK";
	}
}

int main(int argc, char** argv)
{
	if(argc > 1)
	{
		std::vector<uint8_t> buf = read_file(argv[1]);
		model::fmt_stat sb = model::stat(buf.data(), buf.size());
		if(sb.fmt == fmt::none) exit(EXIT_FAILURE);
		printf("[NFO][FMT][%03s] %hu %s\n", fmt_c_str(sb.fmt), sb.cnt, argv[1]);
		opt::vid::tex::h = sb.th;

		glutInit(&argc,argv);
		glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA);
		glutInitWindowSize(opt::vid::w,opt::vid::h);
		glutCreateWindow("Chasm The Rift CAR/3O+ANI Viewer");
		glutDisplayFunc(display);
		glutReshapeFunc(reshape);
		glutKeyboardFunc(keyboard);

		std::vector<path> anim_files;
		for(size_t i = 2; i < argc; i++)
			anim_files.push_back(argv[i]);

		models.push_back(model(argv[1], anim_files));
		for(ani& a : models.back().anis)
			printf(" %s", a.name.c_str());
		std::cout << std::endl;

		glutMainLoop();
		exit(EXIT_SUCCESS);
	}
	exit(EXIT_FAILURE);
}

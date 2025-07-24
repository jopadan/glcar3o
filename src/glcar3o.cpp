#include <chasm/model.hpp>

using namespace chasm;

int main(int argc, char** argv)
{
	if(argc > 1)
	{
		std::vector<path> anim_files;
		for(size_t i = 2; i < argc; i++)
			anim_files.push_back(argv[i]);

		model src(argv[1], anim_files);
		printf("[NFO][MDL][%s] %s", src.fmt == fmt::car ? "CAR" : src.fmt == fmt::c3o ? " 3O" : "UNK", argv[1]);
		for(ani& a : src.anis)
			printf(" %s", a.name.c_str());
		std::cout << std::endl;
		exit(EXIT_SUCCESS);
	}
	exit(EXIT_FAILURE);
}

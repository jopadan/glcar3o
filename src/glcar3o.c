#include <chasm/chasm.h>
#include <error.h>

int main(int argc, char** argv)
{
	if(argc <= 1) exit(EXIT_FAILURE);

	/* load default palette */
	settings.pal = csm_palette_create_fn("chasmpalette.act");

	/* load model */
	model mdl = csm_model_create_fn(argv[1]);

	/* print format info */
	csm_model_format_print(mdl.fmt);

	/* clean up model and palette */
	if(mdl.fmt != CHASM_FORMAT_NONE)
		csm_model_reset(&mdl);
	csm_palette_delete(settings.pal);

	exit(EXIT_SUCCESS);
}

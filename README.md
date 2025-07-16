# glcar3o
[Chasm: The Rift 3D](https://github.com/jopadan/AwesomeChasm/) .[CAR](https://github.com/jopadan/glcar3o/wiki/CAR-CARacter-animation-model-format)/.[3O](https://github.com/jopadan/glcar3o/wiki/3O-3D-Object-Model-Format-Reference) animation model [OpenGL](https://www.gopengl.org/) viewer written in [C23](https://www.open-std.org/jtc1/sc22/wg14/)
## Usage
```sh
./glcar3o assets/hog.car
[NFO][PAL] assets/chasmpalette.act
[NFO][MDL] anim_count: 9 frame_count: 175440
[NFO][FMT] .CAR - Chasm: The Rift CARacter animation model

./glcar3o assets/m-star.3o
[NFO][PAL] assets/chasmpalette.act
[NFO][FMT] .3O  - Chasm: The Rift 3O model
```
## Example
```c
#inclue <chasm/chasm.h>

settings.pal = csm_palette_create_fn("assets/chasmpalette.act");
enum format type = csm_model_format("assets/hog.car");
csm_model_format_print(type);
model hog = csm_model_create_fn("assets/hog.car");
```

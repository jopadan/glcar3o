# glcar3o
Chasm: The Rift 3D .car/.3o animation model OpenGL viewer written in C23

```c
#inclue <chasm/chasm.h>

settings.pal = csm_palette_create_fn("assets/chasmpalette.act");
enum format type = csm_model_format("assets/hog.car");
csm_model_format_print(type);
model hog = csm_model_create_fn("assets/hog.car");
```

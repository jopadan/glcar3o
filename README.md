# glcar3o
[Chasm: The Rift](https://www.mobygames.com/game/2691/chasm-the-rift/) .[CAR](https://github.com/jopadan/glcar3o/wiki/CAR)/.[3O](https://github.com/jopadan/glcar3o/wiki/3O) 3D animation model [OpenGL](https://www.gopengl.org/) viewer written in [C23](https://www.open-std.org/jtc1/sc22/wg14/)

## Usage
```sh
./glcar3o assets/hog.car
[NFO][VID][PAL] assets/chasmpalette.act
[NFO][MDL][CAR] assets/hog.car

./glcar3o assets/m-star.3o assets/m-star.ani
[NFO][VID][PAL] assets/chasmpalette.act
[NFO][MDL][ 3O] assets/m-star.3o assets/m-star.ani

./3oviewer assets/m-star.3o assets/m-star.ani
```
## Example

```c
#inclue <chasm/chasm.h>

using namespace chasm;

int main(int argc, char** argv)
{
    std::vector<path> anim_files;
    for(size_t i = 2; i < argc; i++)
        anim_files.push_back(argv[i]);

    model src(argv[1], anim_files);
    exit(EXIT_SUCCESS);
}
```

## Links

- [Chasm: The Rift](https://www.mobygames.com/game/2691/chasm-the-rift/)
- [AwesomeChasm](https://github.com/jopadan/AwesomeChasm/)
- [The Shadow Zone](https://discord.com/channels/768103789411434586/1374778669612007527)
  - [Chasm Modding Toolkit Package](https://discord.com/channels/768103789411434586/1374842906002718803)
- [meshoptimizer](https://github.com/zeux/meshoptimizer/)

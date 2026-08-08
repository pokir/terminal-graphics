# terminal-graphics

## Setup

```sh
mise install # install dependencies
mise run install # setup git-hooks, etc.
```

Applications can use the standard build rule with a one-line Makefile:

```make
include ../../lib/application.mk
```

For a custom build rule, include `lib/library.mk`. It provides `LIB_HEADERS`,
`LIB_SOURCES`, `LIBRARY`, `INCSPATH`, `LIBS`, and `DEPENDENCIES`. Engine sources
are compiled separately with private dependency headers; application sources
only receive the public `lib/src` include path.

## 3D models

Models can be defined as indexed triangle meshes in C or imported through
Assimp. The first playground build automatically downloads the pinned Assimp
source into `.deps`, builds it locally with at most two compiler jobs, and
caches the result. Nothing is installed system-wide.

`load_model` accepts every importer enabled by Assimp, including OBJ/MTL,
glTF/GLB, FBX, Collada, PLY, STL, 3DS, Blender, and many others. Assimp's full
scene remains retained privately by the model so imported meshes, normals,
UVs, materials, textures, hierarchy, bones, animations, cameras, lights, and
metadata are not discarded or exposed as part of the public interface.

```c
#include "model.h"

Model model;

void setup(void) {
    if (!load_model(&model, "model.glb"))
        /* handle the load error */;
}

void draw(void) {
    mesh(&model, &(ModelTransform){
                     .position = {0., 0., 3.},
                     .rotation = {0., angle_y, 0.},
                     .scale = 1.,
                 });
}

void cleanup(void) {
    free_model(&model);
}
```

## Fonts and text

TTF, OTF, and font-collection files can be rasterized as graphics independently
of the font configured in the terminal:

```c
#include "font.h"

Font font;

void setup(void) {
    load_font(&font, "assets/Inter-Regular.ttf");
}

void draw(void) {
    text(&font, (PixelPos){20, 20}, 48., COLOR_CYAN, "Hello, world!");
}

void cleanup(void) {
    free_font(&font);
}
```

`measure_text` returns the layout size in the same physical-pixel coordinate
space used by the screen API. Text input is UTF-8; available characters depend
on the loaded font.

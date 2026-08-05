# terminal-graphics

## Setup

```sh
mise install # install dependencies
mise run install # setup git-hooks, etc.
```

## 3D models

Models can be defined as indexed triangle meshes in C or loaded from a
Wavefront OBJ file. OBJ polygons are triangulated automatically. Referenced
MTL files are loaded relative to the OBJ file, and each material's diffuse
`Kd` color is applied to faces selected with `usemtl`. JPEG, PNG, and other
common image maps are decoded through the vendored `stb_image` library. Since
the output is a grayscale character grid, image maps contribute their overall
color and material intensity rather than full-resolution texels.

```c
#include "model.h"

Model model;

void setup(void) {
    if (!load_obj(&model, "model.obj"))
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

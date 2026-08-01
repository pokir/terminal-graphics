# terminal-graphics

## Setup

```sh
mise install # install dependencies
mise run install # setup git-hooks, etc.
```

## Notes

user should only implement logic.c (not main.c!)

~~engine uses screen~~: NO! engine does not use screen (engine is just math)
screen uses terminal

TODO: make it so get_terminal_size returns a reference to a pre-calculated size, and recalculate it at the start of each frame in main.c
TODO: make it so terminal drawing (print_at) remembers which characters changed, and only reprint those characters

## 3D models

Models can be defined as indexed triangle meshes in C or loaded from a
Wavefront OBJ file. OBJ polygons are triangulated automatically. Referenced
MTL files are loaded relative to the OBJ file, and each material's diffuse
`Kd` color is applied to faces selected with `usemtl`.

```c
#include "model.h"

Model model;

void setup(void) {
    if (!load_obj(&model, "model.obj"))
        /* handle the load error */;
}

void draw(void) {
    draw_model(&model, (ModelTransform){
                           .position = {0., 0., 3.},
                           .rotation = {0., angle_y, 0.},
                           .scale = 1.,
                       });
}

void cleanup(void) {
    free_model(&model);
}
```

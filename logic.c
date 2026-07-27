#include "logic.h"

#include <stddef.h>

#include "engine.h"
#include "screen.h"

const int TARGET_FPS = 60;
const uint64_t FRAME_TIME_NS = 1000000000ULL / TARGET_FPS;

void setup() {
}

double dz = 0;

void update(double dt) {
  dz += 0.5 * dt;
}

void draw_cube(Pos3D p, double s) {
    const Pos3D points[] = {
        (Pos3D){-s / 2., -s / 2., -s / 2.},
        (Pos3D){ s / 2., -s / 2., -s / 2.},
        (Pos3D){-s / 2.,  s / 2., -s / 2.},
        (Pos3D){ s / 2.,  s / 2., -s / 2.},

        (Pos3D){-s / 2., -s / 2.,  s / 2.},
        (Pos3D){ s / 2., -s / 2.,  s / 2.},
        (Pos3D){-s / 2.,  s / 2.,  s / 2.},
        (Pos3D){ s / 2.,  s / 2.,  s / 2.},
    };

    for (size_t i = 0; i < sizeof(points) / sizeof(points[0]); ++i) {
        Pos3D point = translate(points[i], p);
        pixel(screen(project(point)));
    }
}

void draw() {
  clear();

  Pos3D p = {0, 1, 2 + dz};
  draw_cube(p, 0.9);
}

void cleanup() {
}


#include "logic.h"

#include <stddef.h>

#include "engine.h"
#include "screen.h"

const int TARGET_FPS = 60;
const uint64_t FRAME_TIME_NS = 1000000000ULL / TARGET_FPS;

typedef struct {
    Pos3D pos;
    double angle;
} Cube;

void setup() {
}

double dz = 0;
double angle_y = 0;

void update(double dt) {
    dz += 0.5 * dt;
    angle_y += 3 * dt;
}

void draw_cube(Pos3D p, double s, double angle_y, double angle_z) {
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
        Pos3D point = points[i];
        point = rotate_xz(point, angle_y);
        point = rotate_xy(point, angle_z);
        point = translate(point, p);

        pixel(screen(project(point)));
    }
}

void draw() {
    clear();

    Pos3D p = {0, 0.5, 2 + dz};

    draw_cube(p, 0.9, angle_y, 0);
}

void cleanup() {
}


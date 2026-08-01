#include "logic.h"

#include <stddef.h>

#include "engine.h"
#include "screen.h"

const int TARGET_FPS = 60;

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
    Pos3D points[] = {
        (Pos3D){-s / 2., -s / 2., -s / 2.},
        (Pos3D){ s / 2., -s / 2., -s / 2.},
        (Pos3D){-s / 2.,  s / 2., -s / 2.},
        (Pos3D){ s / 2.,  s / 2., -s / 2.},

        (Pos3D){-s / 2., -s / 2.,  s / 2.},
        (Pos3D){ s / 2., -s / 2.,  s / 2.},
        (Pos3D){-s / 2.,  s / 2.,  s / 2.},
        (Pos3D){ s / 2.,  s / 2.,  s / 2.},
    };

    size_t num_points = sizeof(points) / sizeof(points[0]);

    // transform the points
    for (size_t i = 0; i < num_points; ++i) {
        Pos3D *point = &points[i];
        *point = rotate_xz(*point, angle_y);
        *point = rotate_xy(*point, angle_z);
        *point = translate(*point, p);
    }

    // convert coordinates to screen coordinates
    PixelPos pixel_points[8];
    for (size_t i = 0; i < num_points; ++i) {
        pixel_points[i] = screen(project(points[i]));
    }

    // draw the points
    line(pixel_points[0], pixel_points[1]);
    line(pixel_points[2], pixel_points[3]);
    line(pixel_points[4], pixel_points[5]);
    line(pixel_points[6], pixel_points[7]);

    line(pixel_points[0], pixel_points[2]);
    line(pixel_points[1], pixel_points[3]);
    line(pixel_points[4], pixel_points[6]);
    line(pixel_points[5], pixel_points[7]);

    line(pixel_points[0], pixel_points[4]);
    line(pixel_points[1], pixel_points[5]);
    line(pixel_points[2], pixel_points[6]);
    line(pixel_points[3], pixel_points[7]);
}

void draw() {
    Pos3D p = {0, 0, 1 + 0.00 * dz};
    draw_cube(p, 0.9, angle_y, 0);
}

void cleanup() {
}


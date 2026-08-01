#include "logic.h"

#include <stddef.h>

#include "engine.h"
#include "screen.h"

const int TARGET_FPS = 60;

typedef struct {
    Pos3D pos;
    double angle;
} Cube;

void setup() {}

double dz = 0;
double angle_y = 0;

void update(double dt) {
    dz += 0.5 * dt;
    angle_y += 3 * dt;
}

void draw_cube(Pos3D p, double s, double angle_y, double angle_z) {
    Pos3D points[] = {
        (Pos3D){-s / 2., -s / 2., -s / 2.}, (Pos3D){s / 2., -s / 2., -s / 2.},
        (Pos3D){-s / 2., s / 2., -s / 2.},  (Pos3D){s / 2., s / 2., -s / 2.},

        (Pos3D){-s / 2., -s / 2., s / 2.},  (Pos3D){s / 2., -s / 2., s / 2.},
        (Pos3D){-s / 2., s / 2., s / 2.},   (Pos3D){s / 2., s / 2., s / 2.},
    };

    size_t num_points = sizeof(points) / sizeof(points[0]);

    // transform the points
    for (size_t i = 0; i < num_points; ++i) {
        Pos3D* point = &points[i];
        *point = rotate_xz(*point, angle_y);
        *point = rotate_xy(*point, angle_z);
        *point = translate(*point, p);
    }

    // convert coordinates to screen coordinates
    PixelPos pixel_points[8];
    for (size_t i = 0; i < num_points; ++i) {
        pixel_points[i] = screen(project(points[i]));
    }

    typedef struct {
        int vertices[4];
        Color color;
        double depth;
    } Face;

    Face faces[] = {
        {{0, 1, 3, 2}, COLOR_RED, 0.},  {{4, 6, 7, 5}, COLOR_GREEN, 0.},
        {{0, 2, 6, 4}, COLOR_BLUE, 0.}, {{1, 5, 7, 3}, COLOR_YELLOW, 0.},
        {{0, 4, 5, 1}, COLOR_CYAN, 0.}, {{2, 3, 7, 6}, COLOR_MAGENTA, 0.},
    };
    size_t num_faces = sizeof(faces) / sizeof(faces[0]);

    for (size_t i = 0; i < num_faces; ++i) {
        for (size_t j = 0; j < 4; ++j)
            faces[i].depth += points[faces[i].vertices[j]].z;
        faces[i].depth /= 4.;
    }

    // draw distant faces first so nearer faces cover them
    for (size_t i = 1; i < num_faces; ++i) {
        Face face = faces[i];
        size_t j = i;
        while (j > 0 && faces[j - 1].depth < face.depth) {
            faces[j] = faces[j - 1];
            --j;
        }
        faces[j] = face;
    }

    for (size_t i = 0; i < num_faces; ++i) {
        int* v = faces[i].vertices;
        fill_triangle(pixel_points[v[0]], pixel_points[v[1]],
                      pixel_points[v[2]], faces[i].color);
        fill_triangle(pixel_points[v[0]], pixel_points[v[2]],
                      pixel_points[v[3]], faces[i].color);
    }
}

void draw() {
    Pos3D p = {0, 0, 1 + 0.03 * dz};
    draw_cube(p, 0.9, angle_y, angle_y);
}

void cleanup() {}

#include "logic.h"

#include "model.h"

const int TARGET_FPS = 60;

static Pos3D cube_vertices[] = {
    {-0.5, -0.5, -0.5}, {0.5, -0.5, -0.5}, {-0.5, 0.5, -0.5}, {0.5, 0.5, -0.5},
    {-0.5, -0.5, 0.5},  {0.5, -0.5, 0.5},  {-0.5, 0.5, 0.5},  {0.5, 0.5, 0.5},
};

static ModelTriangle cube_triangles[] = {
    {{0, 1, 3}, {255, 0, 0}},   {{0, 3, 2}, {255, 0, 0}},
    {{4, 6, 7}, {0, 255, 0}},   {{4, 7, 5}, {0, 255, 0}},
    {{0, 2, 6}, {0, 0, 255}},   {{0, 6, 4}, {0, 0, 255}},
    {{1, 5, 7}, {255, 255, 0}}, {{1, 7, 3}, {255, 255, 0}},
    {{0, 4, 5}, {0, 255, 255}}, {{0, 5, 1}, {0, 255, 255}},
    {{2, 3, 7}, {255, 0, 255}}, {{2, 7, 6}, {255, 0, 255}},
};

static Model cube = {
    cube_vertices,
    sizeof(cube_vertices) / sizeof(cube_vertices[0]),
    cube_triangles,
    sizeof(cube_triangles) / sizeof(cube_triangles[0]),
};

void setup() {}

double dz = 0;
double angle_y = 0;

void update(double dt) {
    dz += 0.5 * dt;
    angle_y += 3 * dt;
}

void draw_cube(Pos3D position, double scale, double angle_y, double angle_z) {
    draw_model(&cube, (ModelTransform){
                          .position = position,
                          .rotation = {0., angle_y, angle_z},
                          .scale = scale,
                      });
}

void draw() {
    Pos3D p = {0, 0, 1 + 0.05 * dz};
    draw_cube(p, 0.9, angle_y * 0.3, angle_y * 0.3);
}

void cleanup() {}

#include "logic.h"

#include "model.h"
#include "screen.h"

const int TARGET_FPS = 60;

static Model rabbit;

void setup() {
    load_obj(&rabbit, "assets/rabbit/rabbit.obj");
}

double dz = 0;
double angle_y = 0;

void update(double dt) {
    dz += 0.5 * dt;
    angle_y += 3 * dt;
}

void draw() {
    ModelTransform rabbit_transform = {
        {0., 0., 15.}, {2. + 3 * dz, 1. + 2 * dz, 0. + dz}, 1.};

    mesh(&rabbit, &rabbit_transform);
}

void cleanup() {
    free_model(&rabbit);
}

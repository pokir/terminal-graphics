#include "logic.h"

#include <stdio.h>

#include "font.h"
#include "geometry.h"
#include "model.h"
#include "screen.h"

const int TARGET_FPS = 30;

enum {
    FONT_AVENIR,
    FONT_HERCULANUM,
    FONT_KRUNGTHEP,
    FONT_COUNT,
};

static Font fonts[FONT_COUNT];
static Model text_models[FONT_COUNT];
static int fonts_loaded[FONT_COUNT];
static double angle;

static void center_model(Model* model) {
    if (model->vertex_count == 0)
        return;

    Pos3D minimum = model->vertices[0];
    Pos3D maximum = model->vertices[0];
    for (size_t i = 1; i < model->vertex_count; ++i) {
        Pos3D vertex = model->vertices[i];
        if (vertex.x < minimum.x)
            minimum.x = vertex.x;
        if (vertex.y < minimum.y)
            minimum.y = vertex.y;
        if (vertex.z < minimum.z)
            minimum.z = vertex.z;
        if (vertex.x > maximum.x)
            maximum.x = vertex.x;
        if (vertex.y > maximum.y)
            maximum.y = vertex.y;
        if (vertex.z > maximum.z)
            maximum.z = vertex.z;
    }

    Pos3D center = {(minimum.x + maximum.x) / 2., (minimum.y + maximum.y) / 2.,
                    (minimum.z + maximum.z) / 2.};
    for (size_t i = 0; i < model->vertex_count; ++i)
        model->vertices[i] = translate(
            model->vertices[i], (Pos3D){-center.x, -center.y, -center.z});
}

void setup() {
    const char* paths[FONT_COUNT] = {
        "assets/Avenir.ttc",
        "assets/Herculanum.ttf",
        "assets/Krungthep.ttf",
    };
    const ModelColor colors[FONT_COUNT] = {
        {0., 0.75, 1.},
        {1., 0.1, 0.75},
        {1., 0.75, 0.},
    };

    for (int i = 0; i < FONT_COUNT; ++i) {
        fonts_loaded[i] = load_font(&fonts[i], paths[i]);
        if (!fonts_loaded[i]) {
            fprintf(stderr, "Failed to load %s.\n", paths[i]);
            continue;
        }

        text_models[i] = text_geometry(&fonts[i], "3D", 0.2, colors[i]);
        center_model(&text_models[i]);
    }
}

void update(double dt) {
    angle += dt;
}

void draw() {
    const double y_positions[FONT_COUNT] = {1.8, 0., -1.8};

    for (int i = 0; i < FONT_COUNT; ++i) {
        if (text_models[i].triangle_count == 0)
            continue;

        mesh(&text_models[i],
             &(ModelTransform){
                 .position = {0., y_positions[i], 3.},
                 .rotation = {angle * 0.35, angle, angle * 0.15},
                 .scale = 1.575,
             });
    }
}

void cleanup() {
    for (int i = 0; i < FONT_COUNT; ++i) {
        free_model(&text_models[i]);
        if (fonts_loaded[i])
            free_font(&fonts[i]);
    }
}

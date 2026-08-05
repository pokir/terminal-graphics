#pragma once

#include <stddef.h>
#include <stdint.h>

#include "engine.h"

typedef struct {
    double red;
    double green;
    double blue;
} ModelColor;

typedef struct {
    size_t vertices[3];
    ModelColor color;
} ModelTriangle;

typedef struct Model {
    Pos3D* vertices;
    size_t vertex_count;
    ModelTriangle* triangles;
    size_t triangle_count;
} Model;

typedef struct ModelTransform {
    Pos3D position;
    Pos3D rotation;
    double scale;
} ModelTransform;

// Loads positions and polygon faces from a Wavefront OBJ file. Polygon faces
// are triangulated. Referenced MTL libraries and their diffuse (Kd) colors are
// applied per face; white is used when a material is absent or unknown. Image
// maps contribute their overall color to the terminal material. Returns
// nonzero on success and leaves model empty on failure.
int load_obj(Model* model, const char* path);

// Releases data allocated by load_obj.
void free_model(Model* model);

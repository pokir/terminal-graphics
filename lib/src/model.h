#pragma once

#include <stddef.h>

#include "engine.h"
#include "screen.h"

typedef struct {
    size_t vertices[3];
    Color color;
} ModelTriangle;

typedef struct {
    Pos3D* vertices;
    size_t vertex_count;
    ModelTriangle* triangles;
    size_t triangle_count;
} Model;

typedef struct {
    Pos3D position;
    Pos3D rotation;
    double scale;
} ModelTransform;

// Loads positions and polygon faces from a Wavefront OBJ file. Polygon faces
// are triangulated. Referenced MTL libraries and their diffuse (Kd) colors are
// applied per face; white is used when a material is absent or unknown. Texture
// coordinates, normals, and texture maps are ignored. Returns nonzero on
// success and leaves model empty on failure.
int load_obj(Model* model, const char* path);

// Releases data allocated by load_obj.
void free_model(Model* model);

// Draws every visible triangle, from farthest to nearest.
void draw_model(const Model* model, ModelTransform transform);

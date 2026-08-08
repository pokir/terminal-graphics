#pragma once

#include <stddef.h>
#include <stdint.h>

#include "engine.h"

struct aiScene;

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
    // Complete source scene retained by Assimp for imported models. Include
    // <assimp/scene.h> when accessing its meshes, materials, nodes, animations,
    // textures, cameras, lights, skeletons, or metadata. Procedural models
    // leave this NULL.
    const struct aiScene* scene;
} Model;

typedef struct ModelTransform {
    Pos3D position;
    Pos3D rotation;
    double scale;
} ModelTransform;

// Imports any model format supported by the configured Assimp build. The full
// scene is retained in model.scene and a transformed triangle representation
// is generated for mesh(). Returns nonzero on success and leaves model empty
// on failure.
int load_model(Model* model, const char* path);

// Compatibility alias for load_model().
int load_obj(Model* model, const char* path);

// Returns Assimp's diagnostic for the most recent import failure.
const char* model_error(void);

// Releases both imported scenes and generated rendering data.
void free_model(Model* model);

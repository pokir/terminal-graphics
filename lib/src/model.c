#include "model.h"

#include <assimp/cimport.h>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "image.h"
#include "screen.h"

typedef struct {
    PixelPos points[3];
    Color color;
    double depths[3];
} RenderTriangle;

typedef struct {
    double values[4][4];
} Matrix;

struct ModelImplementation {
    const struct aiScene* scene;
};

static double color_component(double value) {
    if (value <= 0.)
        return 0.;
    if (value >= 1.)
        return 1.;
    return value;
}

static char* copy_string(const char* value) {
    size_t size = strlen(value) + 1;
    char* copy = malloc(size);
    if (copy != NULL)
        memcpy(copy, value, size);
    return copy;
}

static char* relative_path(const char* model_path,
                           const char* referenced_path) {
    if (referenced_path[0] == '/' ||
        (referenced_path[0] != '\0' && referenced_path[1] == ':'))
        return copy_string(referenced_path);

    const char* slash = strrchr(model_path, '/');
    size_t directory_length =
        slash == NULL ? 0 : (size_t)(slash - model_path + 1);
    size_t referenced_length = strlen(referenced_path);
    char* path = malloc(directory_length + referenced_length + 1);
    if (path == NULL)
        return NULL;

    memcpy(path, model_path, directory_length);
    memcpy(path + directory_length, referenced_path, referenced_length + 1);
    return path;
}

static Matrix identity_matrix(void) {
    return (Matrix){.values = {
                        {1., 0., 0., 0.},
                        {0., 1., 0., 0.},
                        {0., 0., 1., 0.},
                        {0., 0., 0., 1.},
                    }};
}

static Matrix assimp_matrix(const struct aiMatrix4x4* matrix) {
    return (Matrix){.values = {
                        {matrix->a1, matrix->a2, matrix->a3, matrix->a4},
                        {matrix->b1, matrix->b2, matrix->b3, matrix->b4},
                        {matrix->c1, matrix->c2, matrix->c3, matrix->c4},
                        {matrix->d1, matrix->d2, matrix->d3, matrix->d4},
                    }};
}

static Matrix multiply_matrix(Matrix a, Matrix b) {
    Matrix result = {0};
    for (int row = 0; row < 4; ++row)
        for (int column = 0; column < 4; ++column)
            for (int i = 0; i < 4; ++i)
                result.values[row][column] +=
                    a.values[row][i] * b.values[i][column];
    return result;
}

static Pos3D transform_point(Matrix matrix, struct aiVector3D point) {
    double x = matrix.values[0][0] * point.x + matrix.values[0][1] * point.y +
               matrix.values[0][2] * point.z + matrix.values[0][3];
    double y = matrix.values[1][0] * point.x + matrix.values[1][1] * point.y +
               matrix.values[1][2] * point.z + matrix.values[1][3];
    double z = matrix.values[2][0] * point.x + matrix.values[2][1] * point.y +
               matrix.values[2][2] * point.z + matrix.values[2][3];
    double w = matrix.values[3][0] * point.x + matrix.values[3][1] * point.y +
               matrix.values[3][2] * point.z + matrix.values[3][3];
    if (w != 0. && w != 1.) {
        x /= w;
        y /= w;
        z /= w;
    }
    return (Pos3D){x, y, z};
}

static void count_node(const struct aiScene* scene,
                       const struct aiNode* node,
                       size_t* vertices,
                       size_t* triangles) {
    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        unsigned int mesh_index = node->mMeshes[i];
        if (mesh_index >= scene->mNumMeshes)
            continue;
        const struct aiMesh* mesh = scene->mMeshes[mesh_index];
        *vertices += mesh->mNumVertices;
        *triangles += mesh->mNumFaces;
    }
    for (unsigned int i = 0; i < node->mNumChildren; ++i)
        count_node(scene, node->mChildren[i], vertices, triangles);
}

static ModelColor material_color(const struct aiScene* scene,
                                 unsigned int material_index,
                                 const char* model_path) {
    ModelColor result = {1., 1., 1.};
    if (material_index >= scene->mNumMaterials)
        return result;

    const struct aiMaterial* material = scene->mMaterials[material_index];
    struct aiColor4D color;
    if (aiGetMaterialColor(material, AI_MATKEY_BASE_COLOR, &color) ==
            aiReturn_SUCCESS ||
        aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &color) ==
            aiReturn_SUCCESS)
        result = (ModelColor){color.r, color.g, color.b};

    ai_real opacity = 1.;
    if (aiGetMaterialFloat(material, AI_MATKEY_OPACITY, &opacity) ==
        aiReturn_SUCCESS) {
        result.red *= opacity;
        result.green *= opacity;
        result.blue *= opacity;
    }

    struct aiString texture_path;
    enum aiTextureType texture_type =
        aiGetMaterialTextureCount(material, aiTextureType_BASE_COLOR) > 0
            ? aiTextureType_BASE_COLOR
            : aiTextureType_DIFFUSE;
    if (aiGetMaterialTexture(material, texture_type, 0, &texture_path, NULL,
                             NULL, NULL, NULL, NULL,
                             NULL) == aiReturn_SUCCESS &&
        texture_path.length > 0 && texture_path.data[0] != '*') {
        char* path = relative_path(model_path, texture_path.data);
        ModelColor average;
        if (path != NULL && image_average_color(path, &average)) {
            result.red *= average.red;
            result.green *= average.green;
            result.blue *= average.blue;
        }
        free(path);
    }

    return result;
}

static void flatten_node(Model* model,
                         const struct aiScene* scene,
                         const struct aiNode* node,
                         Matrix parent_transform,
                         const char* model_path,
                         size_t* vertex_offset,
                         size_t* triangle_offset) {
    Matrix node_transform = multiply_matrix(
        parent_transform, assimp_matrix(&node->mTransformation));

    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        unsigned int mesh_index = node->mMeshes[i];
        if (mesh_index >= scene->mNumMeshes)
            continue;
        const struct aiMesh* mesh = scene->mMeshes[mesh_index];
        size_t base = *vertex_offset;
        ModelColor color =
            material_color(scene, mesh->mMaterialIndex, model_path);

        for (unsigned int j = 0; j < mesh->mNumVertices; ++j)
            model->vertices[(*vertex_offset)++] =
                transform_point(node_transform, mesh->mVertices[j]);

        for (unsigned int j = 0; j < mesh->mNumFaces; ++j) {
            const struct aiFace* face = &mesh->mFaces[j];
            if (face->mNumIndices != 3)
                continue;
            model->triangles[(*triangle_offset)++] = (ModelTriangle){
                {base + face->mIndices[0], base + face->mIndices[1],
                 base + face->mIndices[2]},
                color,
            };
        }
    }

    for (unsigned int i = 0; i < node->mNumChildren; ++i)
        flatten_node(model, scene, node->mChildren[i], node_transform,
                     model_path, vertex_offset, triangle_offset);
}

int load_model(Model* model, const char* path) {
    if (model == NULL || path == NULL)
        return 0;

    *model = (Model){0};
    unsigned int flags =
        aiProcess_Triangulate | aiProcess_JoinIdenticalVertices |
        aiProcess_ImproveCacheLocality | aiProcess_SortByPType |
        aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace |
        aiProcess_ValidateDataStructure | aiProcess_FindInvalidData |
        aiProcess_GenBoundingBoxes;
    const struct aiScene* scene = aiImportFile(path, flags);
    if (scene == NULL || scene->mRootNode == NULL || scene->mNumMeshes == 0) {
        if (scene != NULL)
            aiReleaseImport(scene);
        return 0;
    }

    size_t vertex_count = 0;
    size_t triangle_capacity = 0;
    count_node(scene, scene->mRootNode, &vertex_count, &triangle_capacity);
    if (vertex_count == 0 || triangle_capacity == 0) {
        aiReleaseImport(scene);
        return 0;
    }

    Model loaded = {
        .vertices = malloc(vertex_count * sizeof(*loaded.vertices)),
        .triangles = malloc(triangle_capacity * sizeof(*loaded.triangles)),
        .implementation = malloc(sizeof(*loaded.implementation)),
    };
    if (loaded.vertices == NULL || loaded.triangles == NULL ||
        loaded.implementation == NULL) {
        aiReleaseImport(scene);
        free(loaded.vertices);
        free(loaded.triangles);
        free(loaded.implementation);
        return 0;
    }
    loaded.implementation->scene = scene;

    size_t vertex_offset = 0;
    size_t triangle_offset = 0;
    flatten_node(&loaded, scene, scene->mRootNode, identity_matrix(), path,
                 &vertex_offset, &triangle_offset);
    loaded.vertex_count = vertex_offset;
    loaded.triangle_count = triangle_offset;
    if (loaded.vertex_count == 0 || loaded.triangle_count == 0) {
        free_model(&loaded);
        return 0;
    }

    *model = loaded;
    return 1;
}

int load_obj(Model* model, const char* path) {
    return load_model(model, path);
}

const char* model_error(void) {
    return aiGetErrorString();
}

void free_model(Model* model) {
    if (model == NULL)
        return;

    free(model->vertices);
    free(model->triangles);
    if (model->implementation != NULL) {
        aiReleaseImport(model->implementation->scene);
        free(model->implementation);
    }
    *model = (Model){0};
}

static Pos3D transform_vertex(Pos3D vertex, ModelTransform model_transform) {
    return transform(vertex, model_transform.position, model_transform.rotation,
                     model_transform.scale);
}

static Color screen_color(ModelColor color) {
    double red = pow(color_component(color.red), 1. / 2.2);
    double green = pow(color_component(color.green), 1. / 2.2);
    double blue = pow(color_component(color.blue), 1. / 2.2);
    return (Color){(uint8_t)(red * 255. + 0.5), (uint8_t)(green * 255. + 0.5),
                   (uint8_t)(blue * 255. + 0.5)};
}

void mesh(const Model* model, const ModelTransform* transform) {
    if (model == NULL || model->vertices == NULL || model->triangles == NULL ||
        model->vertex_count == 0 || model->triangle_count == 0 ||
        transform == NULL)
        return;

    Pos3D* vertices = malloc(model->vertex_count * sizeof(*vertices));
    RenderTriangle* triangles =
        malloc(model->triangle_count * sizeof(*triangles));
    if (vertices == NULL || triangles == NULL) {
        free(vertices);
        free(triangles);
        return;
    }

    for (size_t i = 0; i < model->vertex_count; ++i)
        vertices[i] = transform_vertex(model->vertices[i], *transform);

    size_t triangle_count = 0;
    for (size_t i = 0; i < model->triangle_count; ++i) {
        ModelTriangle triangle = model->triangles[i];
        if (triangle.vertices[0] >= model->vertex_count ||
            triangle.vertices[1] >= model->vertex_count ||
            triangle.vertices[2] >= model->vertex_count)
            continue;

        Pos3D a = vertices[triangle.vertices[0]];
        Pos3D b = vertices[triangle.vertices[1]];
        Pos3D c = vertices[triangle.vertices[2]];
        if (a.z <= 0.01 || b.z <= 0.01 || c.z <= 0.01)
            continue;

        triangles[triangle_count++] = (RenderTriangle){
            {screen(project(a)), screen(project(b)), screen(project(c))},
            screen_color(triangle.color),
            {a.z, b.z, c.z},
        };
    }

    for (size_t i = 0; i < triangle_count; ++i)
        fill_triangle_at_depth(triangles[i].points[0], triangles[i].depths[0],
                               triangles[i].points[1], triangles[i].depths[1],
                               triangles[i].points[2], triangles[i].depths[2],
                               triangles[i].color);

    free(triangles);
    free(vertices);
}

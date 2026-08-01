#include "model.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    PixelPos points[3];
    Color color;
    double depth;
} RenderTriangle;

static int append_vertex(Model* model, size_t* capacity, Pos3D vertex) {
    if (model->vertex_count == *capacity) {
        size_t next = *capacity == 0 ? 64 : *capacity * 2;
        Pos3D* vertices = realloc(model->vertices, next * sizeof(*vertices));
        if (vertices == NULL)
            return 0;
        model->vertices = vertices;
        *capacity = next;
    }

    model->vertices[model->vertex_count++] = vertex;
    return 1;
}

static int append_triangle(Model* model,
                           size_t* capacity,
                           ModelTriangle triangle) {
    if (model->triangle_count == *capacity) {
        size_t next = *capacity == 0 ? 64 : *capacity * 2;
        ModelTriangle* triangles =
            realloc(model->triangles, next * sizeof(*triangles));
        if (triangles == NULL)
            return 0;
        model->triangles = triangles;
        *capacity = next;
    }

    model->triangles[model->triangle_count++] = triangle;
    return 1;
}

static int parse_index(const char* token, size_t vertex_count, size_t* index) {
    char* end;
    long value = strtol(token, &end, 10);
    if (end == token || value == 0)
        return 0;

    long resolved = value > 0 ? value - 1 : (long)vertex_count + value;
    if (resolved < 0 || (size_t)resolved >= vertex_count)
        return 0;

    *index = (size_t)resolved;
    return 1;
}

static int parse_face(Model* model, size_t* capacity, char* text, Color color) {
    size_t first;
    size_t previous;
    size_t count = 0;

    for (char* token = strtok(text, " \t\r\n"); token != NULL;
         token = strtok(NULL, " \t\r\n")) {
        if (token[0] == '#')
            break;

        size_t current;
        if (!parse_index(token, model->vertex_count, &current))
            return 0;

        if (count == 0)
            first = current;
        else if (count >= 2 &&
                 !append_triangle(
                     model, capacity,
                     (ModelTriangle){{first, previous, current}, color}))
            return 0;

        previous = current;
        ++count;
    }

    return count >= 3;
}

int load_obj(Model* model, const char* path, Color color) {
    if (model == NULL || path == NULL)
        return 0;

    *model = (Model){0};
    FILE* file = fopen(path, "r");
    if (file == NULL)
        return 0;

    Model loaded = {0};
    size_t vertex_capacity = 0;
    size_t triangle_capacity = 0;
    char line[4096];
    int success = 1;

    while (success && fgets(line, sizeof(line), file) != NULL) {
        if (line[0] == 'v' && (line[1] == ' ' || line[1] == '\t')) {
            Pos3D vertex;
            if (sscanf(line + 1, "%lf %lf %lf", &vertex.x, &vertex.y,
                       &vertex.z) != 3 ||
                !append_vertex(&loaded, &vertex_capacity, vertex))
                success = 0;
        } else if (line[0] == 'f' && (line[1] == ' ' || line[1] == '\t')) {
            if (!parse_face(&loaded, &triangle_capacity, line + 1, color))
                success = 0;
        }
    }

    if (ferror(file))
        success = 0;
    fclose(file);

    if (!success || loaded.vertex_count == 0 || loaded.triangle_count == 0) {
        free_model(&loaded);
        return 0;
    }

    *model = loaded;
    return 1;
}

void free_model(Model* model) {
    if (model == NULL)
        return;

    free(model->vertices);
    free(model->triangles);
    *model = (Model){0};
}

static Pos3D transform_vertex(Pos3D vertex, ModelTransform transform) {
    vertex.x *= transform.scale;
    vertex.y *= transform.scale;
    vertex.z *= transform.scale;
    vertex = rotate_yz(vertex, transform.rotation.x);
    vertex = rotate_xz(vertex, transform.rotation.y);
    vertex = rotate_xy(vertex, transform.rotation.z);
    return translate(vertex, transform.position);
}

static int compare_depth(const void* left, const void* right) {
    const RenderTriangle* a = left;
    const RenderTriangle* b = right;
    if (a->depth < b->depth)
        return 1;
    if (a->depth > b->depth)
        return -1;
    return 0;
}

void draw_model(const Model* model, ModelTransform transform) {
    if (model == NULL || model->vertices == NULL || model->triangles == NULL ||
        model->vertex_count == 0 || model->triangle_count == 0)
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
        vertices[i] = transform_vertex(model->vertices[i], transform);

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

        // Near-plane clipping can be added later; for now, reject triangles
        // crossing or behind the camera so projection remains finite.
        if (a.z <= 0.01 || b.z <= 0.01 || c.z <= 0.01)
            continue;

        triangles[triangle_count++] = (RenderTriangle){
            {screen(project(a)), screen(project(b)), screen(project(c))},
            triangle.color,
            (a.z + b.z + c.z) / 3.,
        };
    }

    qsort(triangles, triangle_count, sizeof(*triangles), compare_depth);
    for (size_t i = 0; i < triangle_count; ++i)
        fill_triangle(triangles[i].points[0], triangles[i].points[1],
                      triangles[i].points[2], triangles[i].color);

    free(triangles);
    free(vertices);
}

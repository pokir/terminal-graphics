#include "model.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    PixelPos points[3];
    Color color;
    double depth;
} RenderTriangle;

typedef struct {
    char* name;
    Color color;
} Material;

typedef struct {
    Material* items;
    size_t count;
    size_t capacity;
} MaterialList;

static char* copy_string(const char* value) {
    size_t size = strlen(value) + 1;
    char* copy = malloc(size);
    if (copy != NULL)
        memcpy(copy, value, size);
    return copy;
}

static char* line_value(char* text) {
    while (*text == ' ' || *text == '\t')
        ++text;

    char* comment = strchr(text, '#');
    if (comment != NULL)
        *comment = '\0';

    size_t length = strlen(text);
    while (length > 0 && (text[length - 1] == ' ' || text[length - 1] == '\t' ||
                          text[length - 1] == '\r' || text[length - 1] == '\n'))
        text[--length] = '\0';
    return text;
}

static void free_materials(MaterialList* materials) {
    for (size_t i = 0; i < materials->count; ++i)
        free(materials->items[i].name);
    free(materials->items);
    *materials = (MaterialList){0};
}

static Material* append_material(MaterialList* materials,
                                 const char* name,
                                 Color fallback) {
    if (materials->count == materials->capacity) {
        size_t next = materials->capacity == 0 ? 8 : materials->capacity * 2;
        Material* items = realloc(materials->items, next * sizeof(*items));
        if (items == NULL)
            return NULL;
        materials->items = items;
        materials->capacity = next;
    }

    char* owned_name = copy_string(name);
    if (owned_name == NULL)
        return NULL;

    Material* material = &materials->items[materials->count++];
    *material = (Material){owned_name, fallback};
    return material;
}

static const Material* find_material(const MaterialList* materials,
                                     const char* name) {
    for (size_t i = materials->count; i > 0; --i)
        if (strcmp(materials->items[i - 1].name, name) == 0)
            return &materials->items[i - 1];
    return NULL;
}

static uint8_t color_component(double value) {
    if (value <= 0.)
        return 0;
    if (value >= 1.)
        return 255;
    return (uint8_t)(value * 255. + 0.5);
}

static int load_mtl(MaterialList* materials, const char* path, Color fallback) {
    FILE* file = fopen(path, "r");
    if (file == NULL)
        return 0;

    char line[4096];
    Material* current = NULL;
    int success = 1;

    while (success && fgets(line, sizeof(line), file) != NULL) {
        char* statement = line;
        while (*statement == ' ' || *statement == '\t')
            ++statement;

        if (strncmp(statement, "newmtl", 6) == 0 &&
            (statement[6] == ' ' || statement[6] == '\t')) {
            char* name = line_value(statement + 6);
            if (*name == '\0' ||
                (current = append_material(materials, name, fallback)) == NULL)
                success = 0;
        } else if (current != NULL && strncmp(statement, "Kd", 2) == 0 &&
                   (statement[2] == ' ' || statement[2] == '\t')) {
            double red;
            double green;
            double blue;
            if (sscanf(statement + 2, "%lf %lf %lf", &red, &green, &blue) == 3)
                current->color =
                    (Color){color_component(red), color_component(green),
                            color_component(blue)};
        }
    }

    if (ferror(file))
        success = 0;
    fclose(file);
    return success;
}

static char* relative_path(const char* obj_path, const char* referenced_path) {
    if (referenced_path[0] == '/')
        return copy_string(referenced_path);

    const char* slash = strrchr(obj_path, '/');
    size_t directory_length =
        slash == NULL ? 0 : (size_t)(slash - obj_path + 1);
    size_t referenced_length = strlen(referenced_path);
    char* path = malloc(directory_length + referenced_length + 1);
    if (path == NULL)
        return NULL;

    memcpy(path, obj_path, directory_length);
    memcpy(path + directory_length, referenced_path, referenced_length + 1);
    return path;
}

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

int load_obj(Model* model, const char* path) {
    if (model == NULL || path == NULL)
        return 0;

    *model = (Model){0};
    FILE* file = fopen(path, "r");
    if (file == NULL)
        return 0;

    Model loaded = {0};
    size_t vertex_capacity = 0;
    size_t triangle_capacity = 0;
    MaterialList materials = {0};
    Color current_color = COLOR_WHITE;
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
            if (!parse_face(&loaded, &triangle_capacity, line + 1,
                            current_color))
                success = 0;
        } else if (strncmp(line, "mtllib", 6) == 0 &&
                   (line[6] == ' ' || line[6] == '\t')) {
            char* names = line_value(line + 6);
            for (char* name = strtok(names, " \t"); name != NULL;
                 name = strtok(NULL, " \t")) {
                char* material_path = relative_path(path, name);
                if (material_path == NULL) {
                    success = 0;
                    break;
                }
                // A missing material library is not fatal: faces remain white.
                load_mtl(&materials, material_path, COLOR_WHITE);
                free(material_path);
            }
        } else if (strncmp(line, "usemtl", 6) == 0 &&
                   (line[6] == ' ' || line[6] == '\t')) {
            const Material* material =
                find_material(&materials, line_value(line + 6));
            current_color = material == NULL ? COLOR_WHITE : material->color;
        }
    }

    if (ferror(file))
        success = 0;
    fclose(file);
    free_materials(&materials);

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

#include "geometry.h"

#include <stddef.h>
#include <stdlib.h>

typedef struct {
    Model model;
    size_t vertex_capacity;
    size_t triangle_capacity;
    int failed;
} GeometryBuilder;

static size_t vertex(GeometryBuilder* builder, Pos3D position) {
    if (builder->failed)
        return 0;
    if (builder->model.vertex_count == builder->vertex_capacity) {
        size_t capacity =
            builder->vertex_capacity == 0 ? 64 : builder->vertex_capacity * 2;
        Pos3D* vertices =
            realloc(builder->model.vertices, capacity * sizeof(*vertices));
        if (vertices == NULL) {
            builder->failed = 1;
            return 0;
        }
        builder->model.vertices = vertices;
        builder->vertex_capacity = capacity;
    }
    size_t index = builder->model.vertex_count++;
    builder->model.vertices[index] = position;
    return index;
}

static void triangle(GeometryBuilder* builder,
                     size_t a,
                     size_t b,
                     size_t c,
                     ModelColor color) {
    if (builder->failed)
        return;
    if (builder->model.triangle_count == builder->triangle_capacity) {
        size_t capacity = builder->triangle_capacity == 0
                              ? 64
                              : builder->triangle_capacity * 2;
        ModelTriangle* triangles =
            realloc(builder->model.triangles, capacity * sizeof(*triangles));
        if (triangles == NULL) {
            builder->failed = 1;
            return;
        }
        builder->model.triangles = triangles;
        builder->triangle_capacity = capacity;
    }
    builder->model.triangles[builder->model.triangle_count++] =
        (ModelTriangle){{a, b, c}, color};
}

static void quad(GeometryBuilder* builder,
                 Pos3D a,
                 Pos3D b,
                 Pos3D c,
                 Pos3D d,
                 ModelColor color) {
    size_t ia = vertex(builder, a);
    size_t ib = vertex(builder, b);
    size_t ic = vertex(builder, c);
    size_t id = vertex(builder, d);
    triangle(builder, ia, ib, ic, color);
    triangle(builder, ia, ic, id, color);
}

static Model finish(GeometryBuilder* builder) {
    if (builder->failed) {
        free_model(&builder->model);
        return (Model){0};
    }
    return builder->model;
}

Model triangle_geometry(Pos3D p1, Pos3D p2, Pos3D p3, ModelColor color) {
    GeometryBuilder builder = {0};
    size_t a = vertex(&builder, p1);
    size_t b = vertex(&builder, p2);
    size_t c = vertex(&builder, p3);
    triangle(&builder, a, b, c, color);
    return finish(&builder);
}

Model box_geometry(double width,
                   double height,
                   double depth,
                   const ModelColor face_colors[6]) {
    static const ModelColor defaults[] = {
        {1., 0., 0.}, {0., 1., 0.}, {0., 0., 1.},
        {1., 1., 0.}, {0., 1., 1.}, {1., 0., 1.},
    };
    const ModelColor* colors = face_colors == NULL ? defaults : face_colors;
    double x = width / 2.;
    double y = height / 2.;
    double z = depth / 2.;
    Pos3D p[] = {
        {-x, -y, -z}, {x, -y, -z}, {-x, y, -z}, {x, y, -z},
        {-x, -y, z},  {x, -y, z},  {-x, y, z},  {x, y, z},
    };
    static const int faces[][4] = {
        {0, 1, 3, 2}, {4, 6, 7, 5}, {0, 2, 6, 4},
        {1, 5, 7, 3}, {0, 4, 5, 1}, {2, 3, 7, 6},
    };

    GeometryBuilder builder = {0};
    for (size_t i = 0; i < sizeof(faces) / sizeof(faces[0]); ++i)
        quad(&builder, p[faces[i][0]], p[faces[i][1]], p[faces[i][2]],
             p[faces[i][3]], colors[i]);
    return finish(&builder);
}

Model cube_geometry(double size, const ModelColor face_colors[6]) {
    return box_geometry(size, size, size, face_colors);
}

Model pyramid_geometry(double width,
                       double height,
                       double depth,
                       const ModelColor face_colors[5]) {
    static const ModelColor defaults[] = {
        {1., 0., 0.}, {0., 1., 0.}, {0., 0., 1.}, {1., 1., 0.}, {0., 1., 1.},
    };
    const ModelColor* colors = face_colors == NULL ? defaults : face_colors;
    double x = width / 2.;
    double z = depth / 2.;
    Pos3D base[] = {
        {-x, 0., -z},
        {x, 0., -z},
        {x, 0., z},
        {-x, 0., z},
    };
    Pos3D top = {0., height, 0.};
    GeometryBuilder builder = {0};
    for (size_t i = 0; i < 4; ++i) {
        size_t next = (i + 1) % 4;
        size_t a = vertex(&builder, base[i]);
        size_t b = vertex(&builder, base[next]);
        size_t c = vertex(&builder, top);
        triangle(&builder, a, b, c, colors[i]);
    }
    quad(&builder, base[3], base[2], base[1], base[0], colors[4]);
    return finish(&builder);
}

static int filled(const FontBitmap* bitmap, int x, int y) {
    return x >= 0 && x < bitmap->width && y >= 0 && y < bitmap->height &&
           bitmap->pixels[(size_t)y * bitmap->width + x] >= 128;
}

Model text_geometry(const Font* font,
                    const char* utf8_text,
                    double depth,
                    ModelColor color) {
    const int resolution = 64;
    FontBitmap bitmap = rasterize_text(font, resolution, utf8_text);
    if (bitmap.pixels == NULL)
        return (Model){0};
    if (depth < 0.)
        depth = -depth;

    GeometryBuilder builder = {0};
    double half_depth = depth / 2.;
    double unit = 1. / bitmap.height;
    for (int y = 0; y < bitmap.height; ++y) {
        for (int x = 0; x < bitmap.width; ++x) {
            if (!filled(&bitmap, x, y))
                continue;
            double left = x * unit;
            double right = (x + 1) * unit;
            double top = (bitmap.height - y) * unit;
            double bottom = (bitmap.height - y - 1) * unit;

            quad(&builder, (Pos3D){left, bottom, half_depth},
                 (Pos3D){right, bottom, half_depth},
                 (Pos3D){right, top, half_depth},
                 (Pos3D){left, top, half_depth}, color);
            if (depth == 0.)
                continue;

            quad(&builder, (Pos3D){right, bottom, -half_depth},
                 (Pos3D){left, bottom, -half_depth},
                 (Pos3D){left, top, -half_depth},
                 (Pos3D){right, top, -half_depth}, color);
            if (!filled(&bitmap, x - 1, y))
                quad(&builder, (Pos3D){left, bottom, -half_depth},
                     (Pos3D){left, bottom, half_depth},
                     (Pos3D){left, top, half_depth},
                     (Pos3D){left, top, -half_depth}, color);
            if (!filled(&bitmap, x + 1, y))
                quad(&builder, (Pos3D){right, bottom, half_depth},
                     (Pos3D){right, bottom, -half_depth},
                     (Pos3D){right, top, -half_depth},
                     (Pos3D){right, top, half_depth}, color);
            if (!filled(&bitmap, x, y - 1))
                quad(&builder, (Pos3D){left, top, half_depth},
                     (Pos3D){right, top, half_depth},
                     (Pos3D){right, top, -half_depth},
                     (Pos3D){left, top, -half_depth}, color);
            if (!filled(&bitmap, x, y + 1))
                quad(&builder, (Pos3D){left, bottom, -half_depth},
                     (Pos3D){right, bottom, -half_depth},
                     (Pos3D){right, bottom, half_depth},
                     (Pos3D){left, bottom, half_depth}, color);
        }
    }

    free_font_bitmap(&bitmap);
    return finish(&builder);
}

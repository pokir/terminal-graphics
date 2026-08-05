#pragma once

#include <stdint.h>

typedef struct Font Font;
typedef struct Model Model;
typedef struct ModelTransform ModelTransform;

typedef struct {
    int x;
    int y;
} PixelPos;

typedef struct {
    int width;
    int height;
} PixelSize;

typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} Color;

extern const Color COLOR_BLACK;
extern const Color COLOR_WHITE;
extern const Color COLOR_RED;
extern const Color COLOR_GREEN;
extern const Color COLOR_BLUE;
extern const Color COLOR_YELLOW;
extern const Color COLOR_CYAN;
extern const Color COLOR_MAGENTA;
extern const Color COLOR_GRAY;

PixelSize screen_size(void);

void clear(Color color);

void pixel(PixelPos p, Color color);

void line(PixelPos p1, PixelPos p2, Color color);

void fill_triangle(PixelPos p1, PixelPos p2, PixelPos p3, Color color);
void fill_triangle_at_depth(PixelPos p1,
                            double depth1,
                            PixelPos p2,
                            double depth2,
                            PixelPos p3,
                            double depth3,
                            Color color);

void rectangle(PixelPos p1, PixelPos p2, Color color);

void fill_rectangle(PixelPos p1, PixelPos p2, Color color);

void text(const Font* font,
          PixelPos position,
          double pixel_height,
          Color color,
          const char* utf8_text);

void mesh(const Model* model, const ModelTransform* transform);

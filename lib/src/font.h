#pragma once

#include <stdint.h>

#include "screen.h"

typedef struct FontImplementation FontImplementation;

typedef struct Font {
    FontImplementation* implementation;
} Font;

typedef struct {
    uint8_t* pixels;
    int width;
    int height;
} FontBitmap;

// Loads the first font face from a TTF or OTF file.
int load_font(Font* font, const char* path);
void free_font(Font* font);

// Measures UTF-8 text in the screen's physical pixel coordinate space.
PixelSize measure_text(const Font* font,
                       double pixel_height,
                       const char* utf8_text);

FontBitmap rasterize_text(const Font* font,
                          int pixel_height,
                          const char* utf8_text);
void free_font_bitmap(FontBitmap* bitmap);

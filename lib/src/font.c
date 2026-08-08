#include "font.h"

#include <stdio.h>
#include <stdlib.h>

#include "terminal.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb/stb_truetype.h"

struct FontImplementation {
    unsigned char* data;
    stbtt_fontinfo info;
};

static unsigned int next_codepoint(const char** text) {
    const unsigned char* bytes = (const unsigned char*)*text;
    unsigned int codepoint;
    int length;

    if (bytes[0] < 0x80) {
        codepoint = bytes[0];
        length = 1;
    } else if ((bytes[0] & 0xe0) == 0xc0 && (bytes[1] & 0xc0) == 0x80) {
        codepoint = ((bytes[0] & 0x1f) << 6) | (bytes[1] & 0x3f);
        length = 2;
    } else if ((bytes[0] & 0xf0) == 0xe0 && (bytes[1] & 0xc0) == 0x80 &&
               (bytes[2] & 0xc0) == 0x80) {
        codepoint = ((bytes[0] & 0x0f) << 12) | ((bytes[1] & 0x3f) << 6) |
                    (bytes[2] & 0x3f);
        length = 3;
    } else if ((bytes[0] & 0xf8) == 0xf0 && (bytes[1] & 0xc0) == 0x80 &&
               (bytes[2] & 0xc0) == 0x80 && (bytes[3] & 0xc0) == 0x80) {
        codepoint = ((bytes[0] & 0x07) << 18) | ((bytes[1] & 0x3f) << 12) |
                    ((bytes[2] & 0x3f) << 6) | (bytes[3] & 0x3f);
        length = 4;
    } else {
        codepoint = 0xfffd;
        length = 1;
    }

    *text += length;
    return codepoint;
}

int load_font(Font* font, const char* path) {
    if (font == NULL || path == NULL)
        return 0;
    font->implementation = NULL;

    FILE* file = fopen(path, "rb");
    if (file == NULL)
        return 0;
    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return 0;
    }
    long size = ftell(file);
    if (size <= 0 || fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return 0;
    }

    FontImplementation* implementation = malloc(sizeof(*implementation));
    unsigned char* data = malloc((size_t)size);
    if (implementation == NULL || data == NULL ||
        fread(data, 1, (size_t)size, file) != (size_t)size) {
        free(implementation);
        free(data);
        fclose(file);
        return 0;
    }
    fclose(file);

    int offset = stbtt_GetFontOffsetForIndex(data, 0);
    if (offset < 0 || !stbtt_InitFont(&implementation->info, data, offset)) {
        free(data);
        free(implementation);
        return 0;
    }

    implementation->data = data;
    font->implementation = implementation;
    return 1;
}

void free_font(Font* font) {
    if (font == NULL || font->implementation == NULL)
        return;
    FontImplementation* implementation = font->implementation;
    free(implementation->data);
    free(implementation);
    font->implementation = NULL;
}

static double subpixel_height(double pixel_height, TerminalSize size) {
    return pixel_height * size.height * TERMINAL_SUBPIXEL_GRID /
           size.height_in_pixels;
}

PixelSize measure_text(const Font* font,
                       double pixel_height,
                       const char* utf8_text) {
    if (font == NULL || font->implementation == NULL || utf8_text == NULL ||
        pixel_height <= 0.)
        return (PixelSize){0, 0};

    TerminalSize terminal = get_terminal_size();
    FontImplementation* implementation = font->implementation;
    float scale = stbtt_ScaleForPixelHeight(
        &implementation->info, (float)subpixel_height(pixel_height, terminal));
    double width = 0.;
    const char* text = utf8_text;
    while (*text != '\0') {
        unsigned int codepoint = next_codepoint(&text);
        int advance;
        stbtt_GetCodepointHMetrics(&implementation->info, (int)codepoint,
                                   &advance, NULL);
        width += advance * scale;
        if (*text != '\0') {
            const char* next = text;
            unsigned int next_value = next_codepoint(&next);
            width +=
                stbtt_GetCodepointKernAdvance(&implementation->info,
                                              (int)codepoint, (int)next_value) *
                scale;
        }
    }

    int physical_width = (int)(width * terminal.width_in_pixels /
                                   (terminal.width * TERMINAL_SUBPIXEL_GRID) +
                               0.5);
    return (PixelSize){physical_width, (int)(pixel_height + 0.5)};
}

FontBitmap rasterize_text(const Font* font,
                          int pixel_height,
                          const char* utf8_text) {
    if (font == NULL || font->implementation == NULL || utf8_text == NULL ||
        pixel_height <= 0)
        return (FontBitmap){0};

    FontImplementation* implementation = font->implementation;
    float scale =
        stbtt_ScaleForPixelHeight(&implementation->info, (float)pixel_height);
    double width = 0.;
    const char* cursor = utf8_text;
    while (*cursor != '\0') {
        unsigned int codepoint = next_codepoint(&cursor);
        int advance;
        stbtt_GetCodepointHMetrics(&implementation->info, (int)codepoint,
                                   &advance, NULL);
        width += advance * scale;
        if (*cursor != '\0') {
            const char* next = cursor;
            unsigned int next_value = next_codepoint(&next);
            width +=
                stbtt_GetCodepointKernAdvance(&implementation->info,
                                              (int)codepoint, (int)next_value) *
                scale;
        }
    }

    int bitmap_width = (int)(width + 1.);
    uint8_t* pixels = calloc((size_t)bitmap_width * pixel_height, 1);
    if (pixels == NULL)
        return (FontBitmap){0};

    int ascent;
    stbtt_GetFontVMetrics(&implementation->info, &ascent, NULL, NULL);
    int baseline = (int)(ascent * scale + 0.5);
    double pen_x = 0.;
    cursor = utf8_text;
    while (*cursor != '\0') {
        unsigned int codepoint = next_codepoint(&cursor);
        int glyph_width;
        int glyph_height;
        int offset_x;
        int offset_y;
        unsigned char* glyph = stbtt_GetCodepointBitmap(
            &implementation->info, scale, scale, (int)codepoint, &glyph_width,
            &glyph_height, &offset_x, &offset_y);
        int destination_x = (int)(pen_x + offset_x + 0.5);
        int destination_y = baseline + offset_y;
        for (int y = 0; y < glyph_height; ++y) {
            int target_y = destination_y + y;
            if (target_y < 0 || target_y >= pixel_height)
                continue;
            for (int x = 0; x < glyph_width; ++x) {
                int target_x = destination_x + x;
                if (target_x < 0 || target_x >= bitmap_width)
                    continue;
                uint8_t alpha = glyph[(size_t)y * glyph_width + x];
                uint8_t* target =
                    &pixels[(size_t)target_y * bitmap_width + target_x];
                if (alpha > *target)
                    *target = alpha;
            }
        }
        stbtt_FreeBitmap(glyph, NULL);

        int advance;
        stbtt_GetCodepointHMetrics(&implementation->info, (int)codepoint,
                                   &advance, NULL);
        pen_x += advance * scale;
        if (*cursor != '\0') {
            const char* next = cursor;
            unsigned int next_value = next_codepoint(&next);
            pen_x +=
                stbtt_GetCodepointKernAdvance(&implementation->info,
                                              (int)codepoint, (int)next_value) *
                scale;
        }
    }

    return (FontBitmap){pixels, bitmap_width, pixel_height};
}

void free_font_bitmap(FontBitmap* bitmap) {
    if (bitmap == NULL)
        return;
    free(bitmap->pixels);
    *bitmap = (FontBitmap){0};
}

void text(const Font* font,
          PixelPos position,
          double pixel_height,
          Color color,
          const char* utf8_text) {
    if (font == NULL || font->implementation == NULL || utf8_text == NULL ||
        pixel_height <= 0.)
        return;

    TerminalSize terminal = get_terminal_size();
    if (terminal.width <= 0 || terminal.height <= 0 ||
        terminal.width_in_pixels <= 0 || terminal.height_in_pixels <= 0)
        return;

    FontImplementation* implementation = font->implementation;
    float scale = stbtt_ScaleForPixelHeight(
        &implementation->info, (float)subpixel_height(pixel_height, terminal));
    int ascent;
    stbtt_GetFontVMetrics(&implementation->info, &ascent, NULL, NULL);
    double origin_x = (double)position.x * terminal.width *
                      TERMINAL_SUBPIXEL_GRID / terminal.width_in_pixels;
    double origin_y = (double)position.y * terminal.height *
                      TERMINAL_SUBPIXEL_GRID / terminal.height_in_pixels;
    double pen_x = origin_x;
    int baseline = (int)(origin_y + ascent * scale + 0.5);

    const char* text = utf8_text;
    while (*text != '\0') {
        unsigned int codepoint = next_codepoint(&text);
        int width;
        int height;
        int offset_x;
        int offset_y;
        unsigned char* bitmap = stbtt_GetCodepointBitmap(
            &implementation->info, scale, scale, (int)codepoint, &width,
            &height, &offset_x, &offset_y);
        int bitmap_x = (int)(pen_x + offset_x + 0.5);
        int bitmap_y = baseline + offset_y;

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int subpixel_x = bitmap_x + x;
                int subpixel_y = bitmap_y + y;
                int cell_x = subpixel_x / TERMINAL_SUBPIXEL_GRID;
                int cell_y = subpixel_y / TERMINAL_SUBPIXEL_GRID;
                int sample_x = subpixel_x % TERMINAL_SUBPIXEL_GRID;
                int sample_y = subpixel_y % TERMINAL_SUBPIXEL_GRID;
                if (sample_x < 0) {
                    sample_x += TERMINAL_SUBPIXEL_GRID;
                    --cell_x;
                }
                if (sample_y < 0) {
                    sample_y += TERMINAL_SUBPIXEL_GRID;
                    --cell_y;
                }
                blend_rgb_subpixel_at((TerminalCharPos){cell_x, cell_y},
                                      sample_x, sample_y, color.red,
                                      color.green, color.blue,
                                      bitmap[(size_t)y * width + x]);
            }
        }
        stbtt_FreeBitmap(bitmap, NULL);

        int advance;
        stbtt_GetCodepointHMetrics(&implementation->info, (int)codepoint,
                                   &advance, NULL);
        pen_x += advance * scale;
        if (*text != '\0') {
            const char* next = text;
            unsigned int next_value = next_codepoint(&next);
            pen_x +=
                stbtt_GetCodepointKernAdvance(&implementation->info,
                                              (int)codepoint, (int)next_value) *
                scale;
        }
    }
}

#include "logic.h"

#include <stdio.h>

#include "font.h"
#include "screen.h"

const int TARGET_FPS = 30;

enum {
    FONT_AVENIR,
    FONT_HERCULANUM,
    FONT_KRUNGTHEP,
    FONT_COUNT,
};

static Font fonts[FONT_COUNT];
static int fonts_loaded[FONT_COUNT];

void setup() {
    const char* paths[FONT_COUNT] = {
        "assets/Avenir.ttc",
        "assets/Herculanum.ttf",
        "assets/Krungthep.ttf",
    };

    for (int i = 0; i < FONT_COUNT; ++i) {
        fonts_loaded[i] = load_font(&fonts[i], paths[i]);
        if (!fonts_loaded[i])
            fprintf(stderr, "Failed to load %s.\n", paths[i]);
    }
}

void update(double dt) {
    (void)dt;
}

static void centered_text(const Font* font,
                          const char* value,
                          int y,
                          double height,
                          Color color) {
    PixelSize canvas = screen_size();
    PixelSize measured = measure_text(font, height, value);
    text(font, (PixelPos){(canvas.width - measured.width) / 2, y}, height,
         color, value);
}

void draw() {
    if (fonts_loaded[FONT_AVENIR]) {
        centered_text(&fonts[FONT_AVENIR], "Terminal Graphics", 20, 240.,
                      COLOR_CYAN);
        centered_text(&fonts[FONT_AVENIR], "UTF-8: café · Zürich · λ", 572, 96.,
                      COLOR_GREEN);
    }
    if (fonts_loaded[FONT_HERCULANUM])
        centered_text(&fonts[FONT_HERCULANUM], "Truecolor RGB text", 280, 144.,
                      COLOR_MAGENTA);
    if (fonts_loaded[FONT_KRUNGTHEP])
        centered_text(&fonts[FONT_KRUNGTHEP], "Kerning: AVATAR", 444, 108.,
                      COLOR_YELLOW);
}

void cleanup() {
    for (int i = 0; i < FONT_COUNT; ++i)
        if (fonts_loaded[i])
            free_font(&fonts[i]);
}

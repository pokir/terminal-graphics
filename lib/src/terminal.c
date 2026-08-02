#include "terminal.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char colored;
} TerminalSubpixel;

typedef struct {
    char glyph;
    TerminalSubpixel subpixels[TERMINAL_SUBPIXEL_GRID][TERMINAL_SUBPIXEL_GRID];
} TerminalCell;

static TerminalCell* screen = NULL;
static int screen_width = 0;
static int screen_height = 0;

static int resize_screen(int width, int height) {
    if (width <= 0 || height <= 0)
        return 0;

    if (width == screen_width && height == screen_height)
        return 1;

    size_t size = (size_t)width * (size_t)height;

    TerminalCell* new_screen = realloc(screen, size * sizeof(*screen));
    if (new_screen == NULL)
        return 0;

    screen = new_screen;
    screen_width = width;
    screen_height = height;

    return 1;
}

void init_renderer(void) {
    // give stdout a large userspace buffer (frame is still flushed once in
    // end_frame) this makes it so newlines ('\n') don't trigger a flush
    setvbuf(stdout, NULL, _IOFBF, 1024 * 1024);

    // Hide the cursor, set an explicit black background for MSAA resolution,
    // and clear the terminal using that background.
    fputs("\033[?25l\033[48;2;0;0;0m\033[2J\033[H", stdout);
}

void begin_frame(void) {
    TerminalSize s = get_terminal_size();

    if (!resize_screen(s.width, s.height))
        return;

    clear_frame();
}

void print_at(TerminalCharPos p, const char* text) {
    if (screen == NULL || text == NULL)
        return;
    if (p.y < 0 || p.y >= screen_height)
        return;

    int x = p.x;

    while (*text != '\0' && x < screen_width) {
        if (x >= 0)
            screen[(size_t)p.y * screen_width + x] =
                (TerminalCell){.glyph = *text};

        ++x;
        ++text;
    }
}

void put_char_at(TerminalCharPos p, char ch) {
    if (screen == NULL)
        return;

    if (p.x < 0 || p.x >= screen_width || p.y < 0 || p.y >= screen_height)
        return;

    screen[(size_t)p.y * screen_width + p.x] = (TerminalCell){.glyph = ch};
}

void put_rgb_at(TerminalCharPos p, uint8_t red, uint8_t green, uint8_t blue) {
    if (screen == NULL)
        return;

    if (p.x < 0 || p.x >= screen_width || p.y < 0 || p.y >= screen_height)
        return;

    TerminalCell* cell = &screen[(size_t)p.y * screen_width + p.x];
    cell->glyph = '#';
    for (int y = 0; y < TERMINAL_SUBPIXEL_GRID; ++y)
        for (int x = 0; x < TERMINAL_SUBPIXEL_GRID; ++x)
            cell->subpixels[y][x] = (TerminalSubpixel){red, green, blue, 1};
}

void put_rgb_subpixel_at(TerminalCharPos p,
                         int subpixel_x,
                         int subpixel_y,
                         uint8_t red,
                         uint8_t green,
                         uint8_t blue) {
    if (screen == NULL || p.x < 0 || p.x >= screen_width || p.y < 0 ||
        p.y >= screen_height || subpixel_x < 0 ||
        subpixel_x >= TERMINAL_SUBPIXEL_GRID || subpixel_y < 0 ||
        subpixel_y >= TERMINAL_SUBPIXEL_GRID)
        return;

    TerminalCell* cell = &screen[(size_t)p.y * screen_width + p.x];
    cell->glyph = '#';
    cell->subpixels[subpixel_y][subpixel_x] =
        (TerminalSubpixel){red, green, blue, 1};
}

void clear_frame(void) {
    if (screen == NULL)
        return;

    size_t size = (size_t)screen_width * (size_t)screen_height;
    for (size_t i = 0; i < size; ++i)
        screen[i] = (TerminalCell){.glyph = ' '};
}

void end_frame(void) {
    if (screen == NULL)
        return;

    // move to the top-left instead of clearing the terminal
    fputs("\033[H", stdout);

    int color_active = 0;
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;

    // Write cells row by row, emitting a truecolor escape only when the color
    // changes instead of once for every character.
    for (int y = 0; y < screen_height; ++y) {
        for (int x = 0; x < screen_width; ++x) {
            TerminalCell cell = screen[(size_t)y * screen_width + x];
            unsigned int resolved_red = 0;
            unsigned int resolved_green = 0;
            unsigned int resolved_blue = 0;
            int coverage = 0;
            for (int sy = 0; sy < TERMINAL_SUBPIXEL_GRID; ++sy) {
                for (int sx = 0; sx < TERMINAL_SUBPIXEL_GRID; ++sx) {
                    TerminalSubpixel sample = cell.subpixels[sy][sx];
                    if (!sample.colored)
                        continue;
                    resolved_red += sample.red;
                    resolved_green += sample.green;
                    resolved_blue += sample.blue;
                    ++coverage;
                }
            }

            const int sample_count =
                TERMINAL_SUBPIXEL_GRID * TERMINAL_SUBPIXEL_GRID;
            resolved_red = (resolved_red + sample_count / 2) / sample_count;
            resolved_green = (resolved_green + sample_count / 2) / sample_count;
            resolved_blue = (resolved_blue + sample_count / 2) / sample_count;

            if (coverage > 0 &&
                (!color_active || resolved_red != red ||
                 resolved_green != green || resolved_blue != blue)) {
                fprintf(stdout, "\033[38;2;%u;%u;%um", resolved_red,
                        resolved_green, resolved_blue);
                color_active = 1;
                red = (uint8_t)resolved_red;
                green = (uint8_t)resolved_green;
                blue = (uint8_t)resolved_blue;
            } else if (coverage == 0 && color_active) {
                // Reset only the foreground; keep the explicit black
                // background active across blank cells and future frames.
                fputs("\033[39m", stdout);
                color_active = 0;
            }
            fputc(cell.glyph, stdout);
        }

        // avoid printing a newline after the final row because that can scroll
        // some terminals
        if (y + 1 < screen_height)
            fputc('\n', stdout);
    }

    if (color_active)
        fputs("\033[39m", stdout);

    fflush(stdout);
}

void shutdown_renderer(void) {
    free(screen);
    screen = NULL;
    screen_width = 0;
    screen_height = 0;

    // restore cursor
    fputs("\033[0m\033[?25h", stdout);
    fflush(stdout);
}

TerminalSize get_terminal_size(void) {
    TerminalSize size = {80, 24, 8 * 80, 16 * 24};  // default fallback values
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) {
        if (ws.ws_col > 0)
            size.width = ws.ws_col;
        if (ws.ws_row > 0)
            size.height = ws.ws_row;
        if (ws.ws_xpixel > 0)
            size.width_in_pixels = ws.ws_xpixel;
        if (ws.ws_ypixel > 0)
            size.height_in_pixels = ws.ws_ypixel;
    }

    return size;
}

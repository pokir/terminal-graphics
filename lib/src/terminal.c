#include "terminal.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* screen = NULL;
static int screen_width = 0;
static int screen_height = 0;

static int resize_screen(int width, int height) {
    if (width <= 0 || height <= 0)
        return 0;

    if (width == screen_width && height == screen_height)
        return 1;

    size_t size = (size_t)width * (size_t)height;

    char* new_screen = realloc(screen, size);
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

    // hide cursor and clear the terminal once
    fputs("\033[?25l\033[2J\033[H", stdout);
}

void begin_frame(void) {
    TerminalSize s = get_terminal_size();

    if (!resize_screen(s.width, s.height))
        return;

    memset(screen, ' ', (size_t)screen_width * (size_t)screen_height);
}

void print_at(TerminalCharPos p, const char* text) {
    if (screen == NULL || text == NULL)
        return;
    if (p.y < 0 || p.y >= screen_height)
        return;

    int x = p.x;

    while (*text != '\0' && x < screen_width) {
        if (x >= 0)
            screen[(size_t)p.y * screen_width + x] = *text;

        ++x;
        ++text;
    }
}

void put_char_at(TerminalCharPos p, char ch) {
    if (screen == NULL)
        return;

    if (p.x < 0 || p.x >= screen_width || p.y < 0 || p.y >= screen_height)
        return;

    screen[(size_t)p.y * screen_width + p.x] = ch;
}

void clear_frame(void) {
    if (screen == NULL)
        return;

    memset(screen, ' ', (size_t)screen_width * (size_t)screen_height);
}

void end_frame(void) {
    if (screen == NULL)
        return;

    // move to the top-left instead of clearing the terminal
    fputs("\033[H", stdout);

    // write to stdout row by row (to add '\n' separators)
    for (int y = 0; y < screen_height; ++y) {
        fwrite(screen + (size_t)y * screen_width, 1, (size_t)screen_width,
               stdout);

        // avoid printing a newline after the final row because that can scroll
        // some terminals
        if (y + 1 < screen_height)
            fputc('\n', stdout);
    }

    fflush(stdout);
}

void shutdown_renderer(void) {
    free(screen);
    screen = NULL;
    screen_width = 0;
    screen_height = 0;

    // restore cursor
    fputs("\033[?25h", stdout);
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

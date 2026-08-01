#include "terminal.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

void clear_screen(void) {
    printf("\033[2J\033[1;1H");
    fflush(stdout);
}

void hide_cursor(void) {
    printf("\033[?25l");
    fflush(stdout);
}

void show_cursor(void) {
    printf("\033[?25h");
    fflush(stdout);
}

void print_at(TerminalCharPos p, const char* text) {
    // terminal coords are 1-based
    printf("\033[%d;%dH%s", p.y + 1, p.x + 1, text);
    fflush(stdout);
}

TerminalSize get_terminal_size(void) {
    TerminalSize size = {80, 24, 8 * 80, 16 * 24}; // default fallback values
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


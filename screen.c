#include "screen.h"

#include "terminal.h"

void clear() {
    clear_screen();
}

void pixel(Pos2D p) {
    TerminalSize s = get_terminal_size();

    int x = p.x * s.width / s.width_in_pixels;
    int y = p.y * s.height / s.height_in_pixels;

    if (x < 0 || x >= s.width || y < 0 || y >= s.height)
        return;

    // terminal coords are 1-based
    print_at(1 + x, 1 + y, "#");
}

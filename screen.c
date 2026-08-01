#include "screen.h"

#include <math.h>
#include <stdlib.h>

#include "terminal.h"

void clear() {
    clear_frame();
}

static TerminalCharPos pixel_to_cell_coords(PixelPos p) {
    TerminalSize s = get_terminal_size();

    int x = (double)p.x * s.width / s.width_in_pixels;
    int y = (double)p.y * s.height / s.height_in_pixels;

    return (TerminalCharPos){x, y};
}

void pixel(PixelPos p) {
    TerminalCharPos tcp = pixel_to_cell_coords(p);

    TerminalSize s = get_terminal_size();
    if (tcp.x < 0 || tcp.x >= s.width || tcp.y < 0 || tcp.y >= s.height)
        return;

    print_at(tcp, "#");
}

void line(PixelPos p1, PixelPos p2) {
    TerminalCharPos tcp1 = pixel_to_cell_coords(p1);
    TerminalCharPos tcp2 = pixel_to_cell_coords(p2);

    int x0 = tcp1.x;
    int y0 = tcp1.y;
    int x1 = tcp2.x;
    int y1 = tcp2.y;

    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);

    int step_x = (x0 < x1) ? 1 : -1;
    int step_y = (y0 < y1) ? 1 : -1;

    int error = dx - dy;

    TerminalSize s = get_terminal_size();

    for (;;) {
        if (x0 >= 0 && x0 < s.width && y0 >= 0 && y0 < s.height) {
            TerminalCharPos tcp = {x0, y0};
            print_at(tcp, "#");
        }

        if (x0 == x1 && y0 == y1)
            break;

        int doubled_error = 2 * error;

        if (doubled_error > -dy) {
            error -= dy;
            x0 += step_x;
        }

        if (doubled_error < dx) {
            error += dx;
            y0 += step_y;
        }
    }
}

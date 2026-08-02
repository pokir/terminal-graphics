#include "screen.h"

#include <math.h>
#include <stdlib.h>

#include "terminal.h"

const Color COLOR_BLACK = {0, 0, 0};
const Color COLOR_WHITE = {255, 255, 255};
const Color COLOR_RED = {255, 0, 0};
const Color COLOR_GREEN = {0, 255, 0};
const Color COLOR_BLUE = {0, 0, 255};
const Color COLOR_YELLOW = {255, 255, 0};
const Color COLOR_CYAN = {0, 255, 255};
const Color COLOR_MAGENTA = {255, 0, 255};
const Color COLOR_GRAY = {128, 128, 128};

static int valid_terminal_size(TerminalSize size) {
    return size.width > 0 && size.height > 0 && size.width_in_pixels > 0 &&
           size.height_in_pixels > 0;
}

PixelSize screen_size(void) {
    TerminalSize size = get_terminal_size();
    return (PixelSize){size.width_in_pixels, size.height_in_pixels};
}

static void put_color_at(TerminalCharPos p, Color color) {
    put_rgb_at(p, color.red, color.green, color.blue);
}

void clear(Color color) {
    TerminalSize size = get_terminal_size();
    if (!valid_terminal_size(size))
        return;

    for (int y = 0; y < size.height; ++y)
        for (int x = 0; x < size.width; ++x)
            put_color_at((TerminalCharPos){x, y}, color);
}

static TerminalCharPos pixel_to_cell(PixelPos p, TerminalSize size) {
    int x = (int)floor((double)p.x * size.width / size.width_in_pixels);
    int y = (int)floor((double)p.y * size.height / size.height_in_pixels);

    return (TerminalCharPos){x, y};
}

static int cell_is_visible(TerminalCharPos p, TerminalSize size) {
    return p.x >= 0 && p.x < size.width && p.y >= 0 && p.y < size.height;
}

void pixel(PixelPos p, Color color) {
    TerminalSize size = get_terminal_size();
    if (!valid_terminal_size(size))
        return;

    TerminalCharPos cell = pixel_to_cell(p, size);
    if (cell_is_visible(cell, size))
        put_color_at(cell, color);
}

static int clip_edge(double p, double q, double* start, double* end) {
    if (p == 0.)
        return q >= 0.;

    double ratio = q / p;
    if (p < 0.) {
        if (ratio > *end)
            return 0;
        if (ratio > *start)
            *start = ratio;
    } else {
        if (ratio < *start)
            return 0;
        if (ratio < *end)
            *end = ratio;
    }

    return 1;
}

// Liang-Barsky clipping keeps a line with distant off-screen endpoints from
// spending thousands of iterations walking toward the visible framebuffer
static int clip_line(TerminalCharPos* p1,
                     TerminalCharPos* p2,
                     TerminalSize size) {
    double x0 = p1->x;
    double y0 = p1->y;
    double dx = (double)p2->x - x0;
    double dy = (double)p2->y - y0;
    double start = 0.;
    double end = 1.;

    if (!clip_edge(-dx, x0, &start, &end) ||
        !clip_edge(dx, size.width - 1. - x0, &start, &end) ||
        !clip_edge(-dy, y0, &start, &end) ||
        !clip_edge(dy, size.height - 1. - y0, &start, &end))
        return 0;

    p2->x = (int)lround(x0 + end * dx);
    p2->y = (int)lround(y0 + end * dy);
    p1->x = (int)lround(x0 + start * dx);
    p1->y = (int)lround(y0 + start * dy);
    return 1;
}

void line(PixelPos p1, PixelPos p2, Color color) {
    TerminalSize size = get_terminal_size();
    if (!valid_terminal_size(size))
        return;

    TerminalCharPos cell1 = pixel_to_cell(p1, size);
    TerminalCharPos cell2 = pixel_to_cell(p2, size);
    if (!clip_line(&cell1, &cell2, size))
        return;

    int x0 = cell1.x;
    int y0 = cell1.y;
    int x1 = cell2.x;
    int y1 = cell2.y;

    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);

    int step_x = (x0 < x1) ? 1 : -1;
    int step_y = (y0 < y1) ? 1 : -1;

    int error = dx - dy;
    for (;;) {
        put_color_at((TerminalCharPos){x0, y0}, color);

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

static long long edge(TerminalCharPos a, TerminalCharPos b, int x, int y) {
    return (long long)(x - a.x) * (b.y - a.y) -
           (long long)(y - a.y) * (b.x - a.x);
}

void fill_triangle(PixelPos p1, PixelPos p2, PixelPos p3, Color color) {
    TerminalSize size = get_terminal_size();
    if (!valid_terminal_size(size))
        return;

    TerminalCharPos a = pixel_to_cell(p1, size);
    TerminalCharPos b = pixel_to_cell(p2, size);
    TerminalCharPos c = pixel_to_cell(p3, size);

    int left = a.x;
    int right = a.x;
    int top = a.y;
    int bottom = a.y;
    TerminalCharPos points[] = {b, c};
    for (size_t i = 0; i < sizeof(points) / sizeof(points[0]); ++i) {
        if (points[i].x < left)
            left = points[i].x;
        if (points[i].x > right)
            right = points[i].x;
        if (points[i].y < top)
            top = points[i].y;
        if (points[i].y > bottom)
            bottom = points[i].y;
    }

    if (right < 0 || left >= size.width || bottom < 0 || top >= size.height)
        return;
    if (left < 0)
        left = 0;
    if (right >= size.width)
        right = size.width - 1;
    if (top < 0)
        top = 0;
    if (bottom >= size.height)
        bottom = size.height - 1;

    long long area = edge(a, b, c.x, c.y);
    if (area == 0) {
        line(p1, p2, color);
        line(p2, p3, color);
        line(p3, p1, color);
        return;
    }

    for (int y = top; y <= bottom; ++y) {
        for (int x = left; x <= right; ++x) {
            long long ab = edge(a, b, x, y);
            long long bc = edge(b, c, x, y);
            long long ca = edge(c, a, x, y);
            if ((ab >= 0 && bc >= 0 && ca >= 0) ||
                (ab <= 0 && bc <= 0 && ca <= 0))
                put_color_at((TerminalCharPos){x, y}, color);
        }
    }
}

void rectangle(PixelPos p1, PixelPos p2, Color color) {
    PixelPos top_left = {p1.x < p2.x ? p1.x : p2.x, p1.y < p2.y ? p1.y : p2.y};
    PixelPos bottom_right = {p1.x > p2.x ? p1.x : p2.x,
                             p1.y > p2.y ? p1.y : p2.y};
    PixelPos top_right = {bottom_right.x, top_left.y};
    PixelPos bottom_left = {top_left.x, bottom_right.y};

    line(top_left, top_right, color);
    line(top_right, bottom_right, color);
    line(bottom_right, bottom_left, color);
    line(bottom_left, top_left, color);
}

void fill_rectangle(PixelPos p1, PixelPos p2, Color color) {
    TerminalSize size = get_terminal_size();
    if (!valid_terminal_size(size))
        return;

    TerminalCharPos cell1 = pixel_to_cell(p1, size);
    TerminalCharPos cell2 = pixel_to_cell(p2, size);
    int left = cell1.x < cell2.x ? cell1.x : cell2.x;
    int right = cell1.x > cell2.x ? cell1.x : cell2.x;
    int top = cell1.y < cell2.y ? cell1.y : cell2.y;
    int bottom = cell1.y > cell2.y ? cell1.y : cell2.y;

    if (left < 0)
        left = 0;
    if (right >= size.width)
        right = size.width - 1;
    if (top < 0)
        top = 0;
    if (bottom >= size.height)
        bottom = size.height - 1;

    for (int y = top; y <= bottom; ++y)
        for (int x = left; x <= right; ++x)
            put_color_at((TerminalCharPos){x, y}, color);
}

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

static double triangle_edge(double ax,
                            double ay,
                            double bx,
                            double by,
                            double x,
                            double y) {
    return (x - ax) * (by - ay) - (y - ay) * (bx - ax);
}

static void fill_triangle_internal(PixelPos p1,
                                   double depth1,
                                   PixelPos p2,
                                   double depth2,
                                   PixelPos p3,
                                   double depth3,
                                   Color color,
                                   int depth_test) {
    TerminalSize size = get_terminal_size();
    if (!valid_terminal_size(size))
        return;

    double ax = (double)p1.x * size.width / size.width_in_pixels;
    double ay = (double)p1.y * size.height / size.height_in_pixels;
    double bx = (double)p2.x * size.width / size.width_in_pixels;
    double by = (double)p2.y * size.height / size.height_in_pixels;
    double cx = (double)p3.x * size.width / size.width_in_pixels;
    double cy = (double)p3.y * size.height / size.height_in_pixels;

    int left = (int)floor(fmin(ax, fmin(bx, cx)));
    int right = (int)floor(fmax(ax, fmax(bx, cx)));
    int top = (int)floor(fmin(ay, fmin(by, cy)));
    int bottom = (int)floor(fmax(ay, fmax(by, cy)));

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

    double area = triangle_edge(ax, ay, bx, by, cx, cy);
    if (area == 0.) {
        line(p1, p2, color);
        line(p2, p3, color);
        line(p3, p1, color);
        return;
    }

    for (int y = top; y <= bottom; ++y) {
        for (int x = left; x <= right; ++x) {
            for (int sy = 0; sy < TERMINAL_SUBPIXEL_GRID; ++sy) {
                for (int sx = 0; sx < TERMINAL_SUBPIXEL_GRID; ++sx) {
                    double sample_x = x + (sx + 0.5) / TERMINAL_SUBPIXEL_GRID;
                    double sample_y = y + (sy + 0.5) / TERMINAL_SUBPIXEL_GRID;
                    double ab =
                        triangle_edge(ax, ay, bx, by, sample_x, sample_y);
                    double bc =
                        triangle_edge(bx, by, cx, cy, sample_x, sample_y);
                    double ca =
                        triangle_edge(cx, cy, ax, ay, sample_x, sample_y);
                    if ((ab >= 0. && bc >= 0. && ca >= 0.) ||
                        (ab <= 0. && bc <= 0. && ca <= 0.)) {
                        if (depth_test) {
                            double weight1 = bc / area;
                            double weight2 = ca / area;
                            double weight3 = ab / area;
                            double inverse_depth = weight1 / depth1 +
                                                   weight2 / depth2 +
                                                   weight3 / depth3;
                            if (inverse_depth > 0.)
                                put_rgb_subpixel_at_depth(
                                    (TerminalCharPos){x, y}, sx, sy,
                                    1. / inverse_depth, color.red, color.green,
                                    color.blue);
                        } else {
                            put_rgb_subpixel_at((TerminalCharPos){x, y}, sx, sy,
                                                color.red, color.green,
                                                color.blue);
                        }
                    }
                }
            }
        }
    }
}

void fill_triangle(PixelPos p1, PixelPos p2, PixelPos p3, Color color) {
    fill_triangle_internal(p1, 1., p2, 1., p3, 1., color, 0);
}

void fill_triangle_at_depth(PixelPos p1,
                            double depth1,
                            PixelPos p2,
                            double depth2,
                            PixelPos p3,
                            double depth3,
                            Color color) {
    if (depth1 <= 0. || depth2 <= 0. || depth3 <= 0.)
        return;
    fill_triangle_internal(p1, depth1, p2, depth2, p3, depth3, color, 1);
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

#include "engine.h"

#include "terminal.h"

Pos2D project(Pos3D p) {
    return (Pos2D){p.x / p.z, p.y / p.z};
}

Pos2D screen(Pos2D p) {
    TerminalSize s = get_terminal_size();
    int min_size = s.width_in_pixels < s.height_in_pixels
        ? s.width_in_pixels
        : s.height_in_pixels;

    double x = p.x * (min_size / 2.) + s.width_in_pixels / 2.;
    double y = -p.y * (min_size / 2.) + s.height_in_pixels / 2.;

    return (Pos2D){x, y};
}

Pos3D translate(Pos3D p, Pos3D translation) {
    return (Pos3D){p.x + translation.x, p.y + translation.y, p.z + translation.z};
}


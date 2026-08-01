#pragma once

#include "screen.h"

typedef struct {
    double x;
    double y;
} Pos2D;

typedef struct {
    double x;
    double y;
    double z;
} Pos3D;

// convert world coords to normalized screen coords
Pos2D project(Pos3D p);

// convert normalized screen coords to screen pixel coords
PixelPos screen(Pos2D p);

Pos3D translate(Pos3D p, Pos3D translation);
Pos3D rotate_xy(Pos3D p, double angle);
Pos3D rotate_xz(Pos3D p, double angle);
Pos3D rotate_yz(Pos3D p, double angle);

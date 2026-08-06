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

// Converts world coordinates to normalized screen coordinates. The camera is
// at the origin and looks along the positive z axis.
Pos2D project(Pos3D p);

// Converts normalized screen coordinates to screen pixel coordinates while
// preserving the aspect ratio of world-space geometry.
PixelPos screen(Pos2D p);

Pos3D add(Pos3D a, Pos3D b);
Pos3D subtract(Pos3D a, Pos3D b);
Pos3D scale(Pos3D p, double factor);
double dot(Pos3D a, Pos3D b);
Pos3D cross(Pos3D a, Pos3D b);
double length(Pos3D p);
double distance(Pos3D a, Pos3D b);
Pos3D normalize(Pos3D p);
Pos3D lerp(Pos3D a, Pos3D b, double amount);

Pos3D translate(Pos3D p, Pos3D translation);
Pos3D rotate_xy(Pos3D p, double angle);
Pos3D rotate_xz(Pos3D p, double angle);
Pos3D rotate_yz(Pos3D p, double angle);
Pos3D rotate_x(Pos3D p, double angle);
Pos3D rotate_y(Pos3D p, double angle);
Pos3D rotate_z(Pos3D p, double angle);
Pos3D rotate(Pos3D p, Pos3D rotation);
Pos3D transform(Pos3D p, Pos3D position, Pos3D rotation, double uniform_scale);

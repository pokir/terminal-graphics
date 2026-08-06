#include "engine.h"

#include <math.h>

Pos2D project(Pos3D p) {
    return (Pos2D){p.x / p.z, p.y / p.z};
}

PixelPos screen(Pos2D p) {
    PixelSize size = screen_size();
    int min_size = size.width < size.height ? size.width : size.height;

    int x = (int)(p.x * (min_size / 2.) + size.width / 2.);
    int y = (int)(-p.y * (min_size / 2.) + size.height / 2.);

    return (PixelPos){x, y};
}

Pos3D add(Pos3D a, Pos3D b) {
    return (Pos3D){a.x + b.x, a.y + b.y, a.z + b.z};
}

Pos3D subtract(Pos3D a, Pos3D b) {
    return (Pos3D){a.x - b.x, a.y - b.y, a.z - b.z};
}

Pos3D scale(Pos3D p, double factor) {
    return (Pos3D){p.x * factor, p.y * factor, p.z * factor};
}

double dot(Pos3D a, Pos3D b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Pos3D cross(Pos3D a, Pos3D b) {
    return (Pos3D){a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z,
                   a.x * b.y - a.y * b.x};
}

double length(Pos3D p) {
    return hypot(hypot(p.x, p.y), p.z);
}

double distance(Pos3D a, Pos3D b) {
    return length(subtract(a, b));
}

Pos3D normalize(Pos3D p) {
    double magnitude = length(p);
    return magnitude == 0. ? (Pos3D){0., 0., 0.} : scale(p, 1. / magnitude);
}

Pos3D lerp(Pos3D a, Pos3D b, double amount) {
    return add(a, scale(subtract(b, a), amount));
}

Pos3D translate(Pos3D p, Pos3D translation) {
    return add(p, translation);
}

Pos3D rotate_xy(Pos3D p, double angle) {
    double c = cos(angle);
    double s = sin(angle);
    return (Pos3D){p.x * c - p.y * s, p.x * s + p.y * c, p.z};
}

Pos3D rotate_xz(Pos3D p, double angle) {
    double c = cos(angle);
    double s = sin(angle);
    return (Pos3D){p.x * c - p.z * s, p.y, p.x * s + p.z * c};
}

Pos3D rotate_yz(Pos3D p, double angle) {
    double c = cos(angle);
    double s = sin(angle);
    return (Pos3D){p.x, p.y * c + p.z * s, -p.y * s + p.z * c};
}

Pos3D rotate_x(Pos3D p, double angle) {
    return rotate_yz(p, angle);
}

Pos3D rotate_y(Pos3D p, double angle) {
    return rotate_xz(p, angle);
}

Pos3D rotate_z(Pos3D p, double angle) {
    return rotate_xy(p, angle);
}

Pos3D rotate(Pos3D p, Pos3D rotation) {
    p = rotate_x(p, rotation.x);
    p = rotate_y(p, rotation.y);
    return rotate_z(p, rotation.z);
}

Pos3D transform(Pos3D p, Pos3D position, Pos3D rotation, double uniform_scale) {
    return translate(rotate(scale(p, uniform_scale), rotation), position);
}

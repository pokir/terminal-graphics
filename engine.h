#pragma once

typedef struct {
  double x;
  double y;
} Pos2D;

typedef struct {
  double x;
  double y;
  double z;
} Pos3D;

Pos2D project(Pos3D p); // convert world cords to normalized screen coords
Pos2D screen(Pos2D p); // convert normalized screen coords to screen pixel coords
Pos3D translate(Pos3D p, Pos3D translation);


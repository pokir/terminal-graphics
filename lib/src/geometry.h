#pragma once

#include "font.h"
#include "model.h"

Model triangle_geometry(Pos3D p1, Pos3D p2, Pos3D p3, ModelColor color);

// Face colors are front, back, left, right, bottom, and top. Passing NULL
// selects the default six-color palette.
Model box_geometry(double width,
                   double height,
                   double depth,
                   const ModelColor face_colors[6]);
Model cube_geometry(double size, const ModelColor face_colors[6]);

// Colors are the four side faces followed by the base. Passing NULL selects a
// default palette.
Model pyramid_geometry(double width,
                       double height,
                       double depth,
                       const ModelColor face_colors[5]);

// Generates camera-independent glyph geometry one world unit high. A depth of
// zero creates flat text; positive depth creates front, back, and side faces.
Model text_geometry(const Font* font,
                    const char* utf8_text,
                    double depth,
                    ModelColor color);

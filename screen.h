#pragma once

typedef struct {
    int x;
    int y;
} PixelPos;

void clear();
void pixel(PixelPos p);
void line(PixelPos p1, PixelPos p2);

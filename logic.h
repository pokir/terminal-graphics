#pragma once

#include <stdint.h>

extern const int TARGET_FPS;

void setup();
void update(double dt); // dt in seconds
void draw();
void cleanup();

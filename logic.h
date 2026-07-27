#pragma once

#include <stdint.h>

extern const int TARGET_FPS;
extern const uint64_t FRAME_TIME_NS;

void setup();
void update(double dt); // dt in seconds
void draw();
void cleanup();


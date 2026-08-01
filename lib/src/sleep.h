#pragma once

#include <stdint.h>
#include <time.h>

uint64_t get_time_ns(void);
void sleep_ns(uint64_t ns);

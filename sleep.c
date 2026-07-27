#include "sleep.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

uint64_t get_time_ns(void) {
    struct timespec ts;

    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        perror("clock_gettime failed");
        exit(EXIT_FAILURE);
    }

    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

void sleep_ns(uint64_t ns) {
    struct timespec req;
    struct timespec rem;

    req.tv_sec = ns / 1000000000ULL;
    req.tv_nsec = ns % 1000000000ULL;

    while (nanosleep(&req, &rem) == -1) {
        if (errno == EINTR) {
            // interrupted by signal, sleep the remaining time
            req = rem;
        } else {
            perror("nanosleep failed");
            break;
        }
    }
}


#include <signal.h>
#include <stdint.h>
#include <stdlib.h>

#include "logic.h"
#include "sleep.h"
#include "terminal.h"

void start() {
  hide_cursor();
  setup();
}

void end() {
  cleanup();
  show_cursor();
}

void sigint_handler(int i) {
  end();
  exit(i);
}

int main() {
  signal(SIGINT, sigint_handler);

  start();

  for (;;) {
    uint64_t frame_start = get_time_ns();

    update(1. / TARGET_FPS);
    draw();

    uint64_t frame_end = get_time_ns();
    uint64_t elapsed = frame_end - frame_start;

    if (elapsed < FRAME_TIME_NS) {
      // sleep remaining time of the frame
      uint64_t sleep_time = FRAME_TIME_NS - elapsed;
      sleep_ns(sleep_time);
    } else {
      // frame took too long!
    }
  }

  end();

  return 0;
}

#include <signal.h>
#include <stdint.h>
#include <stdlib.h>

#include "logic.h"
#include "sleep.h"
#include "terminal.h"

void start() {
  init_renderer();
  setup(); // user-defined
}

void end() {
  cleanup(); // user-defined
  shutdown_renderer();
}

void sigint_handler(int i) {
  end();
  exit(i);
}

int main() {
  signal(SIGINT, sigint_handler);

  start();

  const double delta_time = 1. / TARGET_FPS;
  uint64_t next_frame = get_time_ns();

  for (;;) {
    next_frame += FRAME_TIME_NS;

    update(delta_time); // user-defined

    begin_frame();
    draw(); // user-defined
    end_frame();

    uint64_t frame_end = get_time_ns();

    // sleep for remaining time of the current frame
    if (frame_end < next_frame) {
      sleep_ns(next_frame - frame_end);
    } else {
      // frame took too long!
      // if far behind, reset the schedule so the program does not
      // render many frames without sleeping while trying to catch up
      if (frame_end - next_frame >= FRAME_TIME_NS)
        next_frame = frame_end;
    }
  }

  end();

  return 0;
}

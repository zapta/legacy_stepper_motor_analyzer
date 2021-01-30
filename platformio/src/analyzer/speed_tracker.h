// A simple class to track time and steps pairs and compute the 
// speed.

#pragma once

#include "acquisition.h"

class SpeedTracker {
 public:
  SpeedTracker() { reset(); }

  void reset() {
    has_data_ = false;
    last_ticks_ = 0;
    last_steps_ = 0;
  }

  // Track a new sample. If a pulse/sec estimation is available
  // it returns true and updates *steps_per_sec. 'steps_per_sec'
  // is ignored if null.
  bool track(const acquisition::State* state, int32_t* steps_per_sec);

 private:
  bool has_data_;
  uint32_t last_ticks_;
  int32_t last_steps_;
};

#include "speed_tracker.h"


bool SpeedTracker::track(const acquisition::State* state,
                         int32_t* steps_per_sec) {
  // First time after reset.
  if (!has_data_) {
    last_ticks_ = state->tick_count;
    last_steps_ = state->full_steps;
    has_data_ = true;
    return false;
  }

  // This should behave well even when tick_count overflows.
  const uint32_t delta_ticks = state->tick_count - last_ticks_;

  // Expects a minimal time tick interval for good accuracy.
  if (delta_ticks < (acquisition::TicksPerSecond / 25)) {
    return false;
  }

  const int32_t delta_steps = state->full_steps - last_steps_;


  // Hopefully this can't overflow. Otherwise need to compute
  // using int64.
  if (steps_per_sec != nullptr) {
    const int32_t result =
        (delta_steps * (int32_t)acquisition::TicksPerSecond) / (int32_t)delta_ticks;
    *steps_per_sec = result;
  }

  // Update for next cycle.
  last_ticks_ = state->tick_count;
  last_steps_ = state->full_steps;

  return true;
}


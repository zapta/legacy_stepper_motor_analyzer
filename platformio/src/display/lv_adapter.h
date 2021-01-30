
#pragma once

#include "stm32f4xx_hal.h"

namespace lv_adapter {

extern void setup();

// For developement.
extern void dump_stats();
extern void start_screen_capture();
extern void stop_screen_capture();


}  // namespace lv_adapter

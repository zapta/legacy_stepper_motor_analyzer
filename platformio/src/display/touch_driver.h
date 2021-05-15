
#pragma once

#include <Arduino.h>

namespace touch_driver {
extern void test();
extern void touch_read_read(uint16_t* x, uint16_t* y, bool* is_pressed);

}  // namespace touch_driver

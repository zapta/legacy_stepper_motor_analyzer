
#pragma once

#include <Arduino.h>


namespace tft_driver {

extern void begin();

extern void fillScreen(uint8_t color);

extern void render_buffer(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                          uint8_t* color8_p);

}  // namespace tft_driver


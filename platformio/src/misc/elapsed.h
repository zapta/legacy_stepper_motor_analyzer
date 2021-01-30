

#pragma once

#include <Arduino.h>

class Elapsed {
 public:
  Elapsed() { reset(); }

  void reset() { start_millis_ = millis(); }

  uint32_t elapsed_millis() { return millis() - start_millis_; }

  void advance(uint32_t interval_millis) { start_millis_ += interval_millis; }

  void set(uint32_t elapsed_millis) {
    start_millis_ = millis() - elapsed_millis;
  }

 private:
  uint32_t start_millis_;
};

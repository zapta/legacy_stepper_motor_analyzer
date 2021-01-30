// First order ADC 12 bit signal filter.

#pragma once

namespace filters {

// K is in the range (0, 1024). The higher the value of K, the more the filter
// smooths the signal. We use fixed point integers for efficiency since
// this filter is used by the acquisition interrut routine.
template <uint32_t k>
class Adc12BitsLowPassFilter {
 public:
  Adc12BitsLowPassFilter() : scaled_12bit_value_(0 << 10) {}

  // Accepts the new 12 bit sample and update and return the new
  // filter values.
  inline uint16_t update(uint16_t adc_12_bit_value) {
    const uint32_t t1 = ((uint32_t)adc_12_bit_value) << 10;
    const uint32_t t2 = (t1 * (1024 - k)) + (scaled_12bit_value_ * k);
    scaled_12bit_value_ = t2 >> 10;
    return scaled_12bit_value_ >> 10;
  }

 private:
  // The current value with additional 10 bits representing the
  // fraction.
  uint32_t scaled_12bit_value_;  // current value << 10
};

}  // namespace filters


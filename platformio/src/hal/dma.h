// ADC/DMA configuraiton.
//
// The ADC sampling is triggered by TIM1 and the sampled value
// is transfered to memory using a DMA channel. This buffering
// makes the interrupt response time less critical.

#pragma once

#include "stm32f4xx_hal.h"

namespace dma {

// Two consecutive half words for ADC in0 and in1 samples
// per the confiuration of the ADC and DMA.
struct AdcPoint {
  uint16_t v1;
  uint16_t v2;
};

// ADC DMA buffers. Each 32 bit word contains a pair <adc2, adc1>
// of uint16_t with 12 bit values of ADC2 and ADC1 respectivly.
//
// Size in 32bit words of each of the two DMA ADC buffers.
constexpr int kDmaAdcPointBufferSize = 100;
// Process kDmaAdcBufferSize 32bit words from here
// on  DMA 'half-complete' interrupt.
extern AdcPoint* const kDmaAdcPointBuffer1;
// Process kDmaAdcBufferSize 32bit words from here
// on  DMA 'full-complete' interrupt.
extern AdcPoint* const kDmaAdcPointBuffer2;

// Call this once during initialization.
void MX_DMA_Init();

}  // namespace dma

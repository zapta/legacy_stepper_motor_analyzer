// DMA initialization and buffers.

#include "dma.h"

#include "adc.h"

namespace dma {

// Enable DMA controller clock
void MX_DMA_Init(void) {
  __HAL_RCC_DMA2_CLK_ENABLE();

  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
}

// From CubeMX generated file  stm32f4xx_it.c.
extern "C" void DMA2_Stream0_IRQHandler(void)
{
  /* USER CODE BEGIN DMA2_Stream0_IRQn 0 */

  /* USER CODE END DMA2_Stream0_IRQn 0 */
  HAL_DMA_IRQHandler(&adc::hdma_adc1);
  /* USER CODE BEGIN DMA2_Stream0_IRQn 1 */

  /* USER CODE END DMA2_Stream0_IRQn 1 */
}


// Two consecutive buffers, each of size kDmaAdcBufferSize.
static AdcPoint adc_point_dma_buffers[2 * kDmaAdcPointBufferSize];

extern AdcPoint* const kDmaAdcPointBuffer1 = &(adc_point_dma_buffers[0]);
extern AdcPoint* const kDmaAdcPointBuffer2 = &(adc_point_dma_buffers[kDmaAdcPointBufferSize]);

}  // namespace dma

// ADC configuration.
// ADC1 is triggered by TIM1 and uses ADC2 as a slave for
// simultaneous sampling. Each pair of values is stored
// in DMA buffer as a 32bit value.


#pragma once

#include "stm32f4xx_hal.h"

#define AIN0_Pin GPIO_PIN_0
#define AIN0_GPIO_Port GPIOB

#define AIN1_Pin GPIO_PIN_1
#define AIN1_GPIO_Port GPIOB

namespace adc {

extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;

void MX_ADC1_Init();

}  // namespace adc


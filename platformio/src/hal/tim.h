// Timer 1 definitions and initialization. The timer
// generates the ADC sampling trigger and also has an output
// pin for debugging.

#pragma once

//#include "stm32f1xx_hal.h"
#include "stm32f4xx_hal.h"

namespace tim {
extern TIM_HandleTypeDef htim1;

// Call this once during initialization.
void MX_TIM1_Init();

}  // namespace tim

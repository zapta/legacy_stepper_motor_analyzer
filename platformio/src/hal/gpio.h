// Definitions and initialization of the pins used.

#pragma once

#include "stm32f4xx_hal.h"

// NOTE: LED1, LED2 are on the 32khz crystal pins but we
// don't run that oscilator and it seems to work.

// Pin assignments below copied from MxCube generated configuration.

#define LED1_Pin GPIO_PIN_13
#define LED1_GPIO_Port GPIOC
#define LED2_Pin GPIO_PIN_14
#define LED2_GPIO_Port GPIOC
#define LED3_Pin GPIO_PIN_15
#define LED3_GPIO_Port GPIOC
#define TFT_WR_Pin GPIO_PIN_0
#define TFT_WR_GPIO_Port GPIOA
#define TFT_RST_Pin GPIO_PIN_1
#define TFT_RST_GPIO_Port GPIOA
#define TFT_D14_Pin GPIO_PIN_2
#define TFT_D14_GPIO_Port GPIOA
#define TFT_D12_Pin GPIO_PIN_3
#define TFT_D12_GPIO_Port GPIOA
#define TFT_D11_Pin GPIO_PIN_4
#define TFT_D11_GPIO_Port GPIOA
#define TFT_D9_Pin GPIO_PIN_5
#define TFT_D9_GPIO_Port GPIOA
#define TFT_D8_Pin GPIO_PIN_6
#define TFT_D8_GPIO_Port GPIOA
#define TFT_D7_Pin GPIO_PIN_7
#define TFT_D7_GPIO_Port GPIOA
#define AIN0_Pin GPIO_PIN_0
#define AIN0_GPIO_Port GPIOB
#define AIN1_Pin GPIO_PIN_1
#define AIN1_GPIO_Port GPIOB
#define TFT_D4_Pin GPIO_PIN_10
#define TFT_D4_GPIO_Port GPIOB
#define TFT_D0_Pin GPIO_PIN_12
#define TFT_D0_GPIO_Port GPIOB
#define TFT_D1_Pin GPIO_PIN_13
#define TFT_D1_GPIO_Port GPIOB
#define TFT_D2_Pin GPIO_PIN_14
#define TFT_D2_GPIO_Port GPIOB
#define TFT_D3_Pin GPIO_PIN_15
#define TFT_D3_GPIO_Port GPIOB
#define TFT_D5_Pin GPIO_PIN_9
#define TFT_D5_GPIO_Port GPIOA
#define TFT_D6_Pin GPIO_PIN_10
#define TFT_D6_GPIO_Port GPIOA
#define TFT_D10_Pin GPIO_PIN_15
#define TFT_D10_GPIO_Port GPIOA
#define TFT_D13_Pin GPIO_PIN_4
#define TFT_D13_GPIO_Port GPIOB
#define TFT_D15_Pin GPIO_PIN_5
#define TFT_D15_GPIO_Port GPIOB
#define TFT_DC_Pin GPIO_PIN_8
#define TFT_DC_GPIO_Port GPIOB
#define TFT_BL_Pin GPIO_PIN_9
#define TFT_BL_GPIO_Port GPIOB


namespace gpio {
  
// Call this once during initialization.
void MX_GPIO_Init();
}  // namespace gpio.


// Fast controls for LED0 output. ACTIVE LOW.

// Fast controls for LED1 output.
#define LED1_ON \
  { LED1_GPIO_Port->BSRR = LED1_Pin; }
#define LED1_OFF \
  { LED1_GPIO_Port->BSRR = (uint32_t)LED1_Pin << 16u; }

// Fast controls for LED2 output.
#define LED2_ON \
  { LED2_GPIO_Port->BSRR = LED2_Pin; }
#define LED2_OFF \
  { LED2_GPIO_Port->BSRR = (uint32_t)LED2_Pin << 16u; }

  // Fast controls for LED2 output.
#define LED3_ON \
  { LED3_GPIO_Port->BSRR = LED3_Pin; }
#define LED3_OFF \
  { LED3_GPIO_Port->BSRR = (uint32_t)LED3_Pin << 16u; }

// Fast controls for TFT_DC output.
#define TFT_DC_HIGH \
  { TFT_DC_GPIO_Port->BSRR = TFT_DC_Pin; }
#define TFT_DC_LOW \
  { TFT_DC_GPIO_Port->BSRR = (uint32_t)TFT_DC_Pin << 16u; }

// Fast controls for TFT_WR output.
#define TFT_WR_HIGH \
  { TFT_WR_GPIO_Port->BSRR = TFT_WR_Pin; }
#define TFT_WR_LOW \
  { TFT_WR_GPIO_Port->BSRR = (uint32_t)TFT_WR_Pin << 16u; }

  // Fast controls for TFT_NRST output.
#define TFT_RST_HIGH \
  { TFT_RST_GPIO_Port->BSRR = TFT_RST_Pin; }
#define TFT_RST_LOW \
  { TFT_RST_GPIO_Port->BSRR = (uint32_t)TFT_RST_Pin << 16u; }

  // Fast controls for TFT_BL output.
#define TFT_BL_HIGH \
  { TFT_BL_GPIO_Port->BSRR = TFT_BL_Pin; }
#define TFT_BL_LOW \
  { TFT_BL_GPIO_Port->BSRR = (uint32_t)TFT_BL_Pin << 16u; }


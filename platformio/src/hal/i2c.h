
#pragma once

#include "stm32f4xx_hal.h"

namespace i2c {
extern I2C_HandleTypeDef hi2c1;

void MX_I2C1_Init();

}  // namespace i2c


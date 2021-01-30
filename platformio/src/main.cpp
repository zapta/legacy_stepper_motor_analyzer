// Stepper Motor Analyzer.

#include <Arduino.h>

#include "analyzer/acquisition.h"
#include "display/lv_adapter.h"
#include "display/tft_driver.h"
#include "display/touch_driver.h"
#include "hal/adc.h"
#include "hal/dma.h"
#include "hal/gpio.h"
#include "hal/i2c.h"
#include "hal/tim.h"
#include "lvgl.h"
#include "misc/config_eeprom.h"
#include "misc/elapsed.h"
#include "misc/memory.h"
#include "ui/screen_manager.h"

// We assume an external crystal clock of 25Mhz. When using
// the env:blackpill_f401ce env, the default HSE_VALUE = 8000000
// should be changed by adding this build flag in platformio.ini:
//
//  -D HSE_VALUE=25000000
//
#if (HSE_VALUE) != 25000000
#error "HSE_VALUE should match crystal clock of 25000000"
#endif

constexpr uint8_t kColor8Blue = 0x3;

// TODO: move the lvgl tick timer to the lv_adapter files.
//
// Doc: https://github.com/stm32duino/wiki/wiki/HardwareTimer-library
HardwareTimer tim2(TIM2);

static Elapsed elapsed_from_last_dump;

static void lvgl_irq_tick() { lv_tick_inc(5); }

void setup() {
  // Init hardware.
  gpio::MX_GPIO_Init();
  i2c::MX_I2C1_Init();
  dma::MX_DMA_Init();
  tim::MX_TIM1_Init();
  adc::MX_ADC1_Init();

  acquisition::Settings settings;
  config_eeprom::read_acquisition_settings(&settings);
  acquisition::setup(settings);

  // Since DMA is in done in 16 bit units (half words), we specify the total
  // count of 16 bit values in buffer1 + buffer2. We cas the buffer point to
  // uint32_t as expected by the API.
  HAL_ADC_Start_DMA(&adc::hadc1, (uint32_t*)dma::kDmaAdcPointBuffer1,
                    dma::kDmaAdcPointBufferSize * 2 * 2);

  // HAL_TIM_PWM_Start(&tim::htim1, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&tim::htim1, TIM_CHANNEL_1);

  elapsed_from_last_dump.set(10000);  // force trigger on first loop

  // Start 5ms lvgl interrupts.
  //
  // NOTE: to output the timer pulses for debugging, replace
  // 'NC' with the port name. For example PA15 for LED3 output.
  tim2.setPWM(1, NC, 200, 10, lvgl_irq_tick);

  // TODO: why do we need to init twice for the TFT to work
  // when powering on?  (Not needed for a warm reset.)
  tft_driver::begin();
  tft_driver::fillScreen(kColor8Blue);
  tft_driver::begin();
  tft_driver::fillScreen(kColor8Blue);

  lv_adapter::setup();

  acquisition::reset_state();

  screen_manager::setup();

  lv_task_handler();

  delay(50);  // let the screen process the data.

  TFT_BL_HIGH;
}

void loop() {
  // LVGL processing and rendering.
  lv_task_handler();

  // Screen updates.
  screen_manager::loop();

  // Heartbeat.
  if (millis() % 3000 < 25) {
    LED1_ON;
  } else {
    LED1_OFF;
  }

  // Periodic report over USB/Serial.
  if (elapsed_from_last_dump.elapsed_millis() > 5000) {
    elapsed_from_last_dump.reset();
    Serial.printf("\nMemory: %d\n", memory::free_memory());
    lv_adapter::dump_stats();
  }
}

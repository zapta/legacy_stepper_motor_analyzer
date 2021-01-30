// See display porting here:
// https://docs.lvgl.io/v7/en/html/porting/display.html

#include "lv_adapter.h"

#include "hal/gpio.h"
#include "lvgl.h"
#include "tft_driver.h"
#include "touch_driver.h"

#if LV_COLOR_DEPTH != 8
#error "Expecting LVGL color depth of 8"
#endif

namespace lv_adapter {

#define MY_DISP_HOR_RES (480)

// A static variable to store the buffers.
static lv_disp_buf_t disp_buf;

// LVGL renders up to this number of pixels at a time.
static constexpr uint32_t kBufferSize = MY_DISP_HOR_RES * 40;

// Static buffer(s). Since we don't use DMA, we use only a
// single buffer and define the second one as NULL.
static lv_color_t buf_1[kBufferSize];

// For developer's usage. Eatables screen capture for 
// documentation. Do not release with this flag set.
static bool screen_capture_enabled = false;

// A handler to print lvgl log messages.
// Log level is configured in lv_conf.h.
static void my_log_cb(lv_log_level_t level, const char* file, uint32_t line,
                      const char* fn_name, const char* dsc) {
  // Send the logs via serial port.
  // TODO: change to switch().
  if (level == LV_LOG_LEVEL_ERROR)
    Serial.print("ERROR: ");
  else if (level == LV_LOG_LEVEL_WARN)
    Serial.print("WARNING: ");
  else if (level == LV_LOG_LEVEL_INFO)
    Serial.print("INFO: ");
  else if (level == LV_LOG_LEVEL_TRACE)
    Serial.print("TRACE: ");

  Serial.print("File: ");
  Serial.print(file);
  Serial.print("#");
  Serial.print(line);
  Serial.print(": ");
  Serial.print(fn_name);
  Serial.print(": ");
  Serial.print(dsc);
  Serial.print("\n");
}

// Used during debugging to dump the screen. Enabled by
// ui::kEnableScreenshots.
static void capture_buffer(const lv_area_t* area, lv_color_t* bfr) {
  const int32_t w_pixels = area->x2 - area->x1 + 1;
  const int32_t h_pixels = area->y2 - area->y1 + 1;

  // x,y are relative to the area.
  for (int y = 0; y < h_pixels; y++) {
    Serial.printf("#%d,%d,%d", area->x1, area->y1 + y, w_pixels);
    uint16_t pending_pixels_count = 0;
    uint8_t pending_pixel_color = 0;
    for (int x = 0; x < w_pixels; x++) {
      uint8_t pixel_color = bfr[(uint32_t)y * w_pixels + x].full;

      // Case 0: no pending.
      if (pending_pixels_count == 0) {
        pending_pixels_count = 1;
        pending_pixel_color = pixel_color;
        continue;
      }

      // Case 1: pending exists, append to pending.
      if (pending_pixel_color == pixel_color) {
        pending_pixels_count++;
        continue;
      }

      // Case 2: pending exist, flushing pending.
      Serial.printf(",%hu:%hx", pending_pixels_count, pending_pixel_color);
      pending_pixel_color = pixel_color;
      pending_pixels_count = 1;
    }
    // Flush end of line.
    if (pending_pixels_count > 0) {
      Serial.printf(",%hu:%hx", pending_pixels_count, pending_pixel_color);
    }
    Serial.println();
    delay(50);
  }
}

// Called by LV_GL to flush a buffer to the display. Per our LVGL config,
// color is uint16_t RGB565.
static void my_flush_cb(lv_disp_drv_t* disp_drv, const lv_area_t* area,
                        lv_color_t* color_p) {
  LED3_ON;

  if (screen_capture_enabled) {
    capture_buffer(area, color_p);
  }

  // Per our lv config settings, LVGL uses 8 bits colors.
  const lv_color8_t* lv_color8 = static_cast<lv_color8_t*>(color_p);
  tft_driver::render_buffer(area->x1, area->y1, area->x2, area->y2,
                            (uint8_t*)lv_color8);

  // IMPORTANT!!! Inform the graphics library that flushing was done.
  lv_disp_flush_ready(disp_drv);
  LED3_OFF;
}

void static init_display_driver() {
  // Initialize `disp_buf` with the buffer. We pass NULL for
  // the second (optional) buffer since we don't use DMA.
  lv_disp_buf_init(&disp_buf, buf_1, NULL, kBufferSize);

  lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);

  // Sets an initialized buffer.
  disp_drv.buffer = &disp_buf;
  // Sets a flush callback to draw to the display.
  disp_drv.flush_cb = my_flush_cb;

  // Register the driver and save the created display objects.
  lv_disp_drv_register(&disp_drv);
}

// This is how LVGL reads the touch screen's status.
bool my_touch_read_cb(lv_indev_drv_t* drv, lv_indev_data_t* data) {
  uint16_t x, y;
  bool is_pressed;
  touch_driver::touch_read_read(&x, &y, &is_pressed);
  data->point.x = x;
  data->point.y = y;
  data->state = is_pressed ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
  // No buffering now so no more data read.
  return false;
}

void static init_touch_driver() {
  lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touch_read_cb;
  // Register the driver in LVGL and save the created input device object.
  // TODO: verify result is not null (error)
  lv_indev_drv_register(&indev_drv);
}

// Called once from main on program start.
void setup() {
  lv_init();

  // lv_log_print_g_cb_t
  lv_log_register_print_cb(my_log_cb);

  init_display_driver();
  init_touch_driver();
}

void dump_stats() {
  lv_mem_monitor_t lv_info;
  lv_mem_monitor(&lv_info);
  Serial.printf(
      "total_size=%u, free_cnt=%u, free_size=%u, free_bigest_size=%u\n",
      lv_info.total_size, lv_info.free_cnt, lv_info.free_size,
      lv_info.free_biggest_size);

  Serial.printf("used_cnt=%u, max_used=%u, used_pct=%hu, frag_pct=%hu\n",
                lv_info.used_cnt, lv_info.max_used, lv_info.used_pct,
                lv_info.frag_pct);
}

// For developer's usage. Dump the current screen.
void start_screen_capture() {
  screen_capture_enabled = true;
  Serial.println("###BEGIN screen capture");
}

void stop_screen_capture() {
  screen_capture_enabled = false;
  Serial.println("###END screen capture");
}

}  // namespace lv_adapter

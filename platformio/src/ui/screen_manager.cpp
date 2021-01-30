#include "screen_manager.h"

#include <Arduino.h>

#include "analyzer/acquisition.h"
#include "current_histogram_screen.h"
#include "display/lv_adapter.h"
#include "home_screen.h"
#include "retraction_chart_screen.h"
#include "settings_screen.h"
#include "signal_capture_screen.h"
#include "speed_gauge_screen.h"
#include "steps_chart_screen.h"
#include "steps_histogram_screen.h"
#include "time_histogram_screen.h"

namespace screen_manager {

// Landing screen upon program start.
static constexpr ScreenId kInitialScreen = SCREEN_HOME;

struct ScreenDesc {
  ScreenId screen_id;
  Screen* screen_ptr;
};

static bool screen_cpature_requested = false;

static HomeScreen home_screen;
static SpeedGaugeScreen speed_gauge_screen;
static StepsChartScreen steps_chart_screen;
static RetractionChartScreen retraction_chart_screen;
static TimeHistogramScreen screen_time_histogram;
static StepsHistorgramScreen steps_histogram_screen;
static SignalCaptureScreen signal_capture_screen;
static CurrentHistogramScreen current_histogram_screen;

// Order here determines screen 'next/previous' order.
static const ScreenDesc screen_table[] = {
    {SCREEN_HOME, &home_screen},
    {SCREEN_SPEED_GAUGE, &speed_gauge_screen},
    {SCREEN_STEPS_CHART, &steps_chart_screen},
    {SCREEN_RETRACTION_CHART, &retraction_chart_screen},
    {SCREEN_TIME_HISTOGRAM, &screen_time_histogram},
    {SCREEN_STEPS_HISTOGRAM, &steps_histogram_screen},
    {SCREEN_CURRENT_HISTOGRAM, &current_histogram_screen},
    {SCREEN_SIGNAL_CAPTURE, &signal_capture_screen},
};
constexpr int kNumScreens = sizeof(screen_table) / sizeof(screen_table[0]);

// Settings screen is not part of the screen sequence.
static SettingsScreen screen_settings;
static const ScreenDesc settings_screen_descriptor = {SCREEN_SETTINGS,
                                                      &screen_settings};

static const ScreenDesc* current_screen_desc = nullptr;

// Finds the screen descriptor in table. Returns null if not found.
static const ScreenDesc* find_screen_desc(ScreenId screen_id, int offset) {
  if (screen_id == SCREEN_SETTINGS) {
    // We ignore the offset since Setting screen is not in the
    // main screen sequence.
    return &settings_screen_descriptor;
  }

  // Try to match to the sequential screens.
  for (int i = 0; i < kNumScreens; i++) {
    if (screen_id == screen_table[i].screen_id) {
      int idx = (i + offset) % kNumScreens;
      if (idx < 0) {
        idx += kNumScreens;
      }
      // Here idx is in [0, kNumScreens).
      return &screen_table[idx];
    }
  }
  return nullptr;
}

static void switch_screen(ScreenId screen_id, int offset) {
  ui_events::clear_pending_events();
  const ScreenId current_screen_id = current_screen_desc->screen_id;
  const ScreenDesc* new_screen_desc = find_screen_desc(screen_id, offset);
  if (new_screen_desc == nullptr ||
      new_screen_desc->screen_id == current_screen_id) {
    return;
  }
  current_screen_desc->screen_ptr->on_unload();
  new_screen_desc->screen_ptr->on_load();
  current_screen_desc = new_screen_desc;
  lv_scr_load(current_screen_desc->screen_ptr->lv_scr());
}

// Common to all screens.
// Returns true if event should also be propagated to the screen.
static bool common_event_handler(ui_events::UiEventId ui_event_id) {
  // Handle at this level
  switch (ui_event_id) {
    case ui_events::UI_EVENT_RESET:
      acquisition::reset_state();
      return true;

    case ui_events::UI_EVENT_PREV_PAGE:
      switch_screen(current_screen_desc->screen_id, -1);
      return false;

    case ui_events::UI_EVENT_HOME_PAGE:
      switch_screen(SCREEN_HOME, 0);
      return false;

    case ui_events::UI_EVENT_NEXT_PAGE:
      switch_screen(current_screen_desc->screen_id, 1);
      return false;

    case ui_events::UI_EVENT_SCREENSHOT:
      request_screen_capture();
      return false;

    case ui_events::UI_EVENT_SETTINGS:
      switch_screen(SCREEN_SETTINGS, 0);
      return false;

    default:
      return true;
  }
}

void setup() {
  // Setup all screens.
  for (int i = 0; i < kNumScreens; i++) {
    screen_table[i].screen_ptr->setup(i + 1);
  }

  screen_settings.setup(0);

  // Select initial screen.
  current_screen_desc = find_screen_desc(kInitialScreen, 0);
  current_screen_desc->screen_ptr->on_load();
  lv_scr_load(current_screen_desc->screen_ptr->lv_scr());
}

void loop() {
  // Handle pending ui event, if any.
  ui_events::UiEventId event_id;
  if (ui_events::consume_event(&event_id)) {
    const bool also_send_to_screen = common_event_handler(event_id);
    if (also_send_to_screen) {
      current_screen_desc->screen_ptr->on_event(event_id);
    }
  }

  // Loop the current screen.
  current_screen_desc->screen_ptr->loop();

  if (screen_cpature_requested) {
    const uint32_t start_millis = millis();
    lv_adapter::start_screen_capture();
    lv_obj_invalidate(lv_scr_act());
    lv_refr_now(NULL);
    lv_adapter::stop_screen_capture();
    screen_cpature_requested = false;
    Serial.printf("Screen dump: %u sec", (millis() - start_millis) / 1000);
  }
};

void request_screen_capture() {
  screen_cpature_requested = true;
  Serial.println("Screenshot requested");
}

}  // namespace screen_manager
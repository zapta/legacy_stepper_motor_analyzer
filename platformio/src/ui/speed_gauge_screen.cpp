#include "speed_gauge_screen.h"

#include <Arduino.h>

#include "analyzer/acquisition.h"
#include "misc/memory.h"
#include "ui.h"

constexpr uint16_t kUpdateIntervalMillis = 75;

// Update speed label only every this number of gauge
// updates. Should allow more time to read the text.
constexpr uint8_t kLabelUpdateRatio = 500 / kUpdateIntervalMillis;

static const ui::GaugeAxisConfig kAxisConfigNormal = {
    .min = -2000, .max = 2000, .major_intervals = 4, .minor_intervals = 5};

static const ui::GaugeAxisConfig kAxisConfigAlternative = {
    .min = -500, .max = 500, .major_intervals = 4, .minor_intervals = 5};

SpeedGaugeScreen::SpeedGaugeScreen(){};

void SpeedGaugeScreen::setup(uint8_t screen_num) {
  ui::create_screen(&screen_);
  ui::create_page_elements(screen_, "STEPS / SEC", screen_num, nullptr);
  ui::create_gauge(screen_, kAxisConfigNormal, ui_events::UI_EVENT_SCALE,
                   &gauge_);
  ui::create_label(screen_, 180, 270, 200, "", ui::kFontLargeNumericFields,
                   LV_LABEL_ALIGN_RIGHT, LV_COLOR_YELLOW, &speed_field_);
};

void SpeedGaugeScreen::on_load() {
  speed_tracker_.reset();
  // This avoids value flicker on screen switch.
  speed_field_.set_text("");
  // Force label update on first guage update.
  label_update_divider_ = kLabelUpdateRatio;
};

void SpeedGaugeScreen::on_unload(){};

void SpeedGaugeScreen::on_event(ui_events::UiEventId ui_event_id) {
  switch (ui_event_id) {
    case ui_events::UI_EVENT_SCALE:
      Serial.println("Gauge scale");
      alternative_scale_ = !alternative_scale_;
      gauge_.set_scale(alternative_scale_ ? kAxisConfigAlternative
                                          : kAxisConfigNormal);
      break;

    // This makes the compiler happy.
    default:
      break;
  }
}

void SpeedGaugeScreen::loop() {
   if (display_update_elapsed_.elapsed_millis() < kUpdateIntervalMillis) {
    return;
  }
  display_update_elapsed_.reset();

  // Sample data and update screen.
  const acquisition::State* state = acquisition::sample_state();

  if (label_update_divider_ < kLabelUpdateRatio) {
    label_update_divider_++;
  }

  // See if we have a speed estimation.
  int32_t steps_per_sec;
  if (!speed_tracker_.track(state, &steps_per_sec)) {
    return;
  }

  lv_gauge_set_value(gauge_.lv_gauge, 0, steps_per_sec);

  if (label_update_divider_ >= kLabelUpdateRatio) {
    speed_field_.set_text_int(steps_per_sec);
    label_update_divider_ = 0;
  }
};
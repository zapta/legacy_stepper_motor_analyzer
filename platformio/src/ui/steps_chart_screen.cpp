#include "steps_chart_screen.h"

#include "analyzer/acquisition.h"
#include "ui.h"
#include "ui_events.h"

// CPU limit is about 15 updates sec.
static constexpr uint8_t kUpdatesPerSecond = 10;

static constexpr uint32_t kUpdateIntervalMillis = 1000 / kUpdatesPerSecond;

// Update the steps field once every N time the chart is updated.
static constexpr int kFieldUpdateRatio = 2;

static const ui::ChartAxisConfigs kAxisConfigsNormal{
    .y_range = {.min = 0, .max = 1000},
    .x = {.labels = "0\n2s\n4s\n6s\n8s\n10s", .num_ticks = 4, .dividers = 4},
    .y = {.labels = "1k\n750\n500\n250\n0", .num_ticks = 3, .dividers = 3}};

static const ui::ChartAxisConfigs kAxisConfigsAlternative{
    .y_range = {.min = 0, .max = 5000},
    .x = {.labels = "0\n2s\n4s\n6s\n8s\n10s", .num_ticks = 4, .dividers = 4},
    .y = {.labels = "5k\n4k\n3k\n2k\n1k\n0", .num_ticks = 4, .dividers = 4}};

StepsChartScreen::StepsChartScreen(){};

void StepsChartScreen::setup(uint8_t screen_num) {
  y_offset_ = 0;
  field_update_divider_ = kFieldUpdateRatio;
  points_buffer_.clear();
  ui::create_screen(&screen_);
  ui::create_page_elements(screen_, "STEPS  CHART", screen_num, nullptr);
  ui::create_chart(screen_, kNumPoints, 1, kAxisConfigsNormal,
                   ui_events::UI_EVENT_SCALE, &chart_);

  ui::create_label(screen_, 110, 120, 293, "", ui::kFontNumericDataFields,
                   LV_LABEL_ALIGN_CENTER, LV_COLOR_YELLOW, &steps_field_);
};

void StepsChartScreen::on_load() {
  // Force display update on first loop.
  display_update_elapsed_.set(kUpdateIntervalMillis + 1);
};

void StepsChartScreen::on_unload(){};

void StepsChartScreen::on_event(ui_events::UiEventId ui_event_id) {
  switch (ui_event_id) {
    case ui_events::UI_EVENT_RESET:
      // TODO: define a clear() method and use also in setup().
      y_offset_ = 0;
      field_update_divider_ = kFieldUpdateRatio;
      steps_field_.set_text("");
      points_buffer_.clear();
      chart_.ser1.clear();
      break;

    case ui_events::UI_EVENT_SCALE:
      alternative_scale_ = !alternative_scale_;
      // Data will be scaled on the next loop().
      chart_.set_scale(alternative_scale_ ? kAxisConfigsAlternative
                                          : kAxisConfigsNormal);
      lv_chart_refresh(chart_.lv_chart);
      ;
    default:
      break;
  }
}

void StepsChartScreen::loop() {
  // We update at a fixed rate.
  if (display_update_elapsed_.elapsed_millis() < kUpdateIntervalMillis) {
    return;
  }
  // Instead of reset() we advance to avoid accomulating
  // an error.
  display_update_elapsed_.advance(kUpdateIntervalMillis);

  const acquisition::State* state = acquisition::sample_state();

  int32_t abs_steps = state->full_steps;

  // Shift the point into our local buffer.
  *points_buffer_.insert() = abs_steps;

  if (++field_update_divider_ >= kFieldUpdateRatio) {
    field_update_divider_ = 0;
    steps_field_.set_text_int(abs_steps);
  }

  const ui::Range& y_range = alternative_scale_
                                 ? kAxisConfigsAlternative.y_range
                                 : kAxisConfigsNormal.y_range;

  // Adjust offset if value got out of chart range.
  const int32_t rel_steps = abs_steps + y_offset_;
  if (rel_steps > y_range.max) {
    y_offset_ -= (rel_steps - y_range.max);
  } else if (rel_steps < y_range.min) {
    y_offset_ += (y_range.min - rel_steps);
  }

  const uint16_t n = points_buffer_.size();

  for (uint16_t i = 0; i < n; i++) {
    const int32_t rel_point_steps = *points_buffer_.get(i) + y_offset_;

    const lv_coord_t chart_val = rel_point_steps;

    lv_chart_set_point_id(chart_.lv_chart, chart_.ser1.lv_series, chart_val, i);
  }

  lv_chart_refresh(chart_.lv_chart);
}
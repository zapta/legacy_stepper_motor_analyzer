#include "retraction_chart_screen.h"

#include "analyzer/acquisition.h"
#include "ui.h"
#include "ui_events.h"

// CPU limit is about 15 updates sec.
static constexpr uint8_t kUpdatesPerSecond = 10;

static constexpr uint32_t kUpdateIntervalMillis = 1000 / kUpdatesPerSecond;

// Update the retraction field once every N times the chart is updated.
static constexpr int kFieldUpdateRatio = 2;

static const ui::ChartAxisConfigs kAxisConfigsNormal{
    .y_range = {.min = 0, .max = 100},
    .x = {.labels = "0\n2s\n4s\n6s\n8s\n10s", .num_ticks = 4, .dividers = 4},
    .y = {.labels = "100\n80\n60\n40\n20\n0", .num_ticks = 4, .dividers = 4}};

static const ui::ChartAxisConfigs kAxisConfigsAlternative{
    .y_range = {.min = 0, .max = 500},
    .x = {.labels = "0\n2s\n4s\n6s\n8s\n10s", .num_ticks = 4, .dividers = 4},
    .y = {
        .labels = "500\n400\n300\n200\n100\n0", .num_ticks = 4, .dividers = 4}};

void RetractionChartScreen::setup(uint8_t screen_num) {
  // y_offset_ = 0;
  field_update_divider_ = kFieldUpdateRatio;
  // points_buffer_.clear();
  ui::create_screen(&screen_);
  ui::create_page_elements(screen_, "RETRACTION CHART", screen_num, nullptr);
  ui::create_chart(screen_, kNumPoints, 1, kAxisConfigsNormal,
                   ui_events::UI_EVENT_SCALE, &chart_);

  ui::create_label(screen_, 110, 120, 293, "", ui::kFontNumericDataFields,
                   LV_LABEL_ALIGN_CENTER, LV_COLOR_YELLOW, &retraction_field_);
};

void RetractionChartScreen::on_load() {
  chart_.ser1.clear();

  // Force display update on first loop.
  display_update_elapsed_.set(kUpdateIntervalMillis + 1);
};

void RetractionChartScreen::on_unload(){};

void RetractionChartScreen::on_event(ui_events::UiEventId ui_event_id) {
  switch (ui_event_id) {
    case ui_events::UI_EVENT_RESET:
      field_update_divider_ = kFieldUpdateRatio;
      retraction_field_.set_text("");

      chart_.ser1.clear();
      break;

    case ui_events::UI_EVENT_SCALE:
      alternative_scale_ = !alternative_scale_;
      chart_.set_scale(alternative_scale_ ? kAxisConfigsAlternative
                                          : kAxisConfigsNormal);
      lv_chart_refresh(chart_.lv_chart);

    default:
      break;
  }
}

void RetractionChartScreen::loop() {
  // We update at a fixed rate.
  if (display_update_elapsed_.elapsed_millis() < kUpdateIntervalMillis) {
    return;
  }
  // Instead of reset() we advance to avoid accomulating
  // an error.
  display_update_elapsed_.advance(kUpdateIntervalMillis);

  const acquisition::State* state = acquisition::sample_state();

  int retraction_steps = state->max_full_steps - state->full_steps;

  if (++field_update_divider_ >= kFieldUpdateRatio) {
    field_update_divider_ = 0;
    retraction_field_.set_text_int(retraction_steps);
  }

  chart_.ser1.set_next((lv_coord_t)retraction_steps);
}
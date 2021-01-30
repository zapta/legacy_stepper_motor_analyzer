#include "steps_histogram_screen.h"

#include "analyzer/acquisition.h"
#include "ui.h"

static constexpr uint32_t kUpdateIntervalMillis = 200;

static const ui::ChartAxisConfigs kAxisConfigsNormal{
    .y_range = {.min = 0, .max = 100},
    .x = {.labels = "0\n500\n1000\n1500\n2000", .num_ticks = 5, .dividers = 3},
    .y = {.labels = "100%\n75%\n50%\n25%\n0", .num_ticks = 5, .dividers = 3}};

StepsHistorgramScreen::StepsHistorgramScreen(){};

void StepsHistorgramScreen::setup(uint8_t screen_num) {
  ui::create_screen(&screen_);
  ui::create_page_elements(screen_, "STEPS BY STEPS/SEC", screen_num, nullptr);
  ui::create_histogram(screen_, acquisition::kNumHistogramBuckets,
                       kAxisConfigsNormal, &histogram_);
};

void StepsHistorgramScreen::on_load() {
  // Force display update on first loop.
  display_update_elapsed_.set(kUpdateIntervalMillis + 1);
};

void StepsHistorgramScreen::on_unload(){};

void StepsHistorgramScreen::on_event(ui_events::UiEventId ui_event_id) {
  switch (ui_event_id) {
    case ui_events::UI_EVENT_RESET:
      acquisition::reset_state();

      break;

    default:
      break;
  }
}

void StepsHistorgramScreen::loop() {
  // We update at a fixed rate.
  if (display_update_elapsed_.elapsed_millis() < kUpdateIntervalMillis) {
    return;
  }
  display_update_elapsed_.reset();

  // Sample data and update screen.
  const acquisition::State* state = acquisition::sample_state();

  // Find max steps in a bucket.
  uint64_t max_steps = state->buckets[0].total_steps;
  for (int i = 1; i < acquisition::kNumHistogramBuckets; i++) {
    uint64_t steps = state->buckets[i].total_steps;
    if (steps > max_steps) {
      max_steps = steps;
    }
  }

  // Update all the histogram points.
  for (int i = 0; i < acquisition::kNumHistogramBuckets; i++) {
    uint64_t steps = state->buckets[i].total_steps;
    // Scale the value to [0, 100];
    uint16_t val = max_steps > 0 ? ((steps * 100) / max_steps) : 0;

    // Indicate non zero buckets. Even if it's a tiny fraction.
    if (steps > 0 && val == 0) {
      val = 1;
    }

    histogram_.lv_series->points[i] = val;
  }

  lv_chart_refresh(histogram_.lv_chart);
}
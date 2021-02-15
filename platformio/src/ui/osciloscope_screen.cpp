#include "osciloscope_screen.h"

#include "analyzer/acquisition.h"
#include "ui.h"

static constexpr uint16_t kCaptureDividerNormal = 10;
static const ui::ChartAxisConfigs kAxisConfigsNormal{
    .y_range = {.min = -2500, .max = 2500},
    .x = {.labels = "0\n5ms\n10ms\n15ms\n20ms", .num_ticks = 5, .dividers = 3},
    .y = {.labels = "2.5A\n0\n-2.5A", .num_ticks = 3, .dividers = 1}};

static constexpr uint16_t kCaptureDividerAlternative = 50;
static const ui::ChartAxisConfigs kAxisConfigsAlternative{
    .y_range = {.min = -2500, .max = 2500},
    .x = {.labels = "0\n20ms\n40ms\n60ms\n80ms\n100ms",
          .num_ticks = 6,
          .dividers = 4},
    .y = {.labels = "2.5A\n0\n-2.5A", .num_ticks = 3, .dividers = 1}};

void OsciloscopeScreen::setup(uint8_t screen_num) {
  ui::create_screen(&screen_);
  ui::create_page_elements(screen_, "CURRENT PATTERNS", screen_num, nullptr);
  ui::create_chart(screen_, acquisition::kCaptureBufferSize, 2,
                   kAxisConfigsNormal, ui_events::UI_EVENT_SCALE, &chart_);
  capture_controls_.setup(screen_);
};

void OsciloscopeScreen::on_load() {
  capture_controls_.sync_button_to_state();
  update_display();
};

void OsciloscopeScreen::on_event(ui_events::UiEventId ui_event_id) {
  switch (ui_event_id) {
    case ui_events::UI_EVENT_RESET:
      capture_util::clear_data();
      capture_controls_.sync_button_to_state();
      update_display();
      break;

    case ui_events::UI_EVENT_SCALE: {
      capture_util::toggle_scale();
      capture_controls_.sync_button_to_state();
      update_display();
    } break;

    // This makes the compiler happy.
    default:
      break;
  }
}

// Update chart from shared state.
void OsciloscopeScreen::update_display() {
  // TODO: can we skip this most of the times? Is it expensive?
  chart_.set_scale(capture_util::alternative_scale() ? kAxisConfigsAlternative
                                                       : kAxisConfigsNormal);
  capture_controls_.update_display_from_state();

  // No capture data.
  if (!capture_util::has_data()) {
    chart_.ser1.clear();
    chart_.ser2.clear();
    lv_chart_refresh(chart_.lv_chart);
    return;
  }

  // Has capture data.
  const acquisition::CaptureBuffer* capture_buffer =  capture_util::capture_buffer();
  for (int i = 0; i < acquisition::kCaptureBufferSize; i++) {
    const acquisition::CaptureItem* item =
        capture_buffer->items.get(i);
    // Currents in millamps [-2000, 2000].
    const int milliamps1 = acquisition::adc_value_to_milliamps(item->v1);
    const int milliamps2 = acquisition::adc_value_to_milliamps(item->v2);

    lv_chart_set_point_id(chart_.lv_chart, chart_.ser1.lv_series, milliamps1,
                          i);
    lv_chart_set_point_id(chart_.lv_chart, chart_.ser2.lv_series, milliamps2,
                          i);
  }

  // Chart is dirty. Mark it for refresh.
  lv_chart_refresh(chart_.lv_chart);
}

void OsciloscopeScreen::loop() {
  // Update capture enabled if needed.
  if ( capture_util::maybe_update_state_from_controls(capture_controls_)) {
    update_display();
    return;
  }

  if (capture_util::maybe_update_capture_data()) {
     update_display();
   }

}
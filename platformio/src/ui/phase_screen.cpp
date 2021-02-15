#include "phase_screen.h"

#include "analyzer/acquisition.h"
#include "ui.h"

// TODO: Make class member? Share with other screen?
static lv_point_t points[acquisition::kCaptureBufferSize];

static const ui::ChartAxisConfigs kAxisConfigs{
    .y_range = {.min = -2500, .max = 2500},
    .x = {.labels = "-2.5A\n0\n2.5A", .num_ticks = 3, .dividers = 1},
    .y = {.labels = "2.5A\n0\n-2.5A", .num_ticks = 3, .dividers = 1}};

void PhaseScreen::setup(uint8_t screen_num) {
  ui::create_screen(&screen_);
  ui::create_page_elements(screen_, "PHASE PATTERNS", screen_num, nullptr);
  ui::create_polar_chart(screen_, kAxisConfigs, &polar_chart_);
  capture_controls_.setup(screen_);
};

void PhaseScreen::on_load() { 
    capture_controls_.sync_button_to_state();

  update_display();
   };

void PhaseScreen::on_event(ui_events::UiEventId ui_event_id) {
  switch (ui_event_id) {
    case ui_events::UI_EVENT_RESET:
      capture_util::clear_data();
      capture_controls_.sync_button_to_state();
      update_display();
      break;

    // This makes the compiler happy.
    default:
      break;
  }
}

// Maps -2500 to +2500 ma to 0 to 2*max_radius.
static lv_coord_t map_line_coord(int milliamps, lv_coord_t max_radius) {
  return (lv_coord_t)(((milliamps + 2500) * max_radius) / 2500);
}

void PhaseScreen::update_display() {
  capture_controls_.update_display_from_state();

  if (!capture_util::has_data()) {
    lv_line_set_points(polar_chart_.lv_line, points, 0);
    return;
  }

  // Update both chart series with the new captured data.
  for (int i = 0; i < acquisition::kCaptureBufferSize; i++) {
    const acquisition::CaptureItem* item =
        capture_util::capture_buffer()->items.get(i);
    // Currents in millamps [-2000, 2000].
    const int milliamps1 = acquisition::adc_value_to_milliamps(item->v1);
    const int milliamps2 = acquisition::adc_value_to_milliamps(item->v2);

    points[i].x = map_line_coord(milliamps1, polar_chart_.max_radius);
    points[i].y = map_line_coord(milliamps2, polar_chart_.max_radius);
  }

  // The line keeps a reference to our points buffer.
  lv_line_set_points(polar_chart_.lv_line, points,
                     acquisition::kCaptureBufferSize);
}

void PhaseScreen::loop() {
  // Update capture enabled from button if needed.
  if (capture_util::maybe_update_state_from_controls(capture_controls_)) {
    update_display();
    return;
  }

  if (capture_util::maybe_update_capture_data()) {
    update_display();
  }
}
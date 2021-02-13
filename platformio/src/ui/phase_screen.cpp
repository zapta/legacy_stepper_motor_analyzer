#include "phase_screen.h"

#include "analyzer/acquisition.h"
#include "ui.h"

static constexpr uint32_t kUpdateIntervalMillis = 500;

static constexpr uint16_t kCaptureDivider = 20;

// TODO: Make class member? Share with other screen?
static lv_point_t points[acquisition::kCaptureBufferSize];

static const ui::ChartAxisConfigs kAxisConfigs{
    .y_range = {.min = -2500, .max = 2500},
    .x = {.labels = "-2.5A\n0\n2.5A", .num_ticks = 3, .dividers = 1},
    .y = {.labels = "2.5A\n0\n-2.5A", .num_ticks = 3, .dividers = 1}};

// Should work even if capture is in progress.
void PhaseScreen::startCapture() {
  acquisition::start_capture(kCaptureDivider);
  capture_in_progress_ = true;
}

void PhaseScreen::setup(uint8_t screen_num) {
  ui::create_screen(&screen_);
  ui::create_page_elements(screen_, "PHASE PATTERNS", screen_num, nullptr);
  ui::create_polar_chart(screen_, kAxisConfigs, &polar_chart_);
};

void PhaseScreen::on_load() {
  clear_chart();
  // Force display update on first loop.
  startCapture();
};

void PhaseScreen::clear_chart() {
   lv_line_set_points(polar_chart_.lv_line, points, 0 );
}

void PhaseScreen::on_event(ui_events::UiEventId ui_event_id) {
  switch (ui_event_id) {
    case ui_events::UI_EVENT_RESET:
      clear_chart();
      break;

    // This makes the compiler happy.
    default:
      break;
  }
}

// Maps -2500 to +2500 ma to 0 to 2*max_radius.
static lv_coord_t map_line_coord(int milliamps,
                                        lv_coord_t max_radius) {
  return (lv_coord_t)  (((milliamps + 2500) * max_radius) / 2500);
}

void PhaseScreen::loop() {
  // Capture is enabled but capture si not in progress.
  if (!capture_in_progress_) {
    if (display_update_elapsed_.elapsed_millis() >= kUpdateIntervalMillis) {
      startCapture();
    }
    return;
  }

  // Capture is enabled and is progress. Return if
  // capture resutls are not ready yet.
  if (!acquisition::is_capture_ready()) {
    return;
  }

  // Here where capture is enablled and the last
  // capture request was just completed.
  capture_in_progress_ = false;

  // Since capture is not in progress now, the content of the
  // capture buffer should be stable and it's safe to use
  // it.
  const acquisition::CaptureBuffer* capture_buffer =
      acquisition::capture_buffer();
  const acquisition::CaptureItems* items = &capture_buffer->items;

  // Currently we display only captured with a trigger point.
  // This seens to give a better user experience.
  if (!capture_buffer->trigger_found) {
    return;
  }

  // Update both chart series with the new captured data.
  for (int i = 0; i < acquisition::kCaptureBufferSize; i++) {
    const acquisition::CaptureItem* item = items->get(i);
    // Currents in millamps [-2000, 2000].
    const int milliamps1 = acquisition::adc_value_to_milliamps(item->v1);
    const int milliamps2 = acquisition::adc_value_to_milliamps(item->v2);

    points[i].x = map_line_coord(milliamps1, polar_chart_.max_radius);
    points[i].y = map_line_coord(milliamps2, polar_chart_.max_radius);
  }

  // The line keeps a reference to our points buffer.
  lv_line_set_points(polar_chart_.lv_line, points,
                     acquisition::kCaptureBufferSize);

  // Reset the display time, This will let the new display stay
  // on screen for a short time before we start a new capture.
  display_update_elapsed_.reset();
}
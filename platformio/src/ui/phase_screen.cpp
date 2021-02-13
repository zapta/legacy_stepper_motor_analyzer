#include "phase_screen.h"

#include "analyzer/acquisition.h"
#include "ui.h"

static constexpr uint32_t kUpdateIntervalMillis = 500;

static constexpr uint16_t kCaptureDivider = 10;

static const ui::ChartAxisConfigs kAxisConfigs{
    .y_range = {.min = -2500, .max = 2500},
    .x = {.labels = "-2.5A\n0\n2.5A", .num_ticks = 3, .dividers = 1},
    .y = {.labels = "2.5A\n0\n-2.5A", .num_ticks = 3, .dividers = 1}};

// static constexpr uint16_t kCaptureDividerAlternative = 50;
// static const ui::ChartAxisConfigs kAxisConfigsAlternative{
//     .y_range = {.min = -2500, .max = 2500},
//     .x = {.labels = "0\n20ms\n40ms\n60ms\n80ms\n100ms",
//           .num_ticks = 6,
//           .dividers = 4},
//     .y = {.labels = "2.5A\n0\n-2.5A", .num_ticks = 3, .dividers = 1}};

// void PhaseScreen::set_displayed_status(Status status) {
//   switch (status) {
//     case RUNNING:
//       status_label_.set_text("RUNNING");
//       status_label_.set_text_color(LV_COLOR_GREEN);
//       run_button_.label.set_text(ui::kSymbolPause);
//       break;

//     case STOPPED:
//       status_label_.set_text("STOPPED");
//       status_label_.set_text_color(LV_COLOR_RED);
//       run_button_.label.set_text(ui::kSymbolRun);
//       break;

//     default:
//       status_label_.set_text("???");
//   }
// }

// Should work even if capture is in progress.
void PhaseScreen::startCapture() {
  acquisition::start_capture(kCaptureDivider);
  capture_in_progress_ = true;
}

void PhaseScreen::setup(uint8_t screen_num) {
  ui::create_screen(&screen_);
  ui::create_page_elements(screen_, "PHASE PATTERNS", screen_num, nullptr);
  ui::create_polar_chart(screen_, kAxisConfigs, &polar_chart_);

  // ui::create_label(screen_, 100, 130, 0, "", ui::kFontPageTitles,
  //                  LV_LABEL_ALIGN_CENTER, LV_COLOR_RED, &status_label_);

  // // Button's symbol is set later by set_displayed_status().
  // ui::create_button(screen_, 45, 150, ui::kBottomButtonsPosY, "",
  // LV_COLOR_GRAY,
  //                   ui_events::UI_EVENT_CAPTURE, &run_button_);

  // lv_btn_set_checkable(run_button_.lv_button, true);

  // lv_obj_set_state(run_button_.lv_button, LV_STATE_CHECKED);
  // set_displayed_status(RUNNING);

};

void PhaseScreen::on_load() {
  // Force display update on first loop.
  startCapture();
  // chart_.ser1.clear();
  // chart_.ser2.clear();
};

void PhaseScreen::clear_chart() {
  // chart_.ser1.clear();
  // chart_.ser2.clear();
  // lv_obj_set_state(run_button_.lv_button, LV_STATE_CHECKED);
  // lv_chart_refresh(chart_.lv_chart);
}

void PhaseScreen::on_event(ui_events::UiEventId ui_event_id) {
  switch (ui_event_id) {
    case ui_events::UI_EVENT_RESET:
      clear_chart();
      break;

    // case ui_events::UI_EVENT_SCALE: {
    //   alternative_scale_ = !alternative_scale_;
    //   // if (scale_alternative_) {
    //   chart_.set_scale(alternative_scale_ ? kAxisConfigsAlternative
    //                                       : kAxisConfigsNormal);
    //   clear_chart();
    //   Serial.printf("Scale event %d\n", alternative_scale_);
    // } break;

    // case ui_events::UI_EVENT_DEBUG: {
    //   // Dump graph to serial out.
    //   Serial.println("\nGraphs:");
    //   for (int i = 0; i < acquisition::kCaptureBufferSize; i++) {
    //     const uint16_t v1 =
    //         lv_chart_get_point_id(chart_.lv_chart, chart_.ser1.lv_series, i);
    //     const uint16_t v2 =
    //         lv_chart_get_point_id(chart_.lv_chart, chart_.ser2.lv_series, i);
    //     Serial.printf("%3d,%6hd,%6hd\n", i, v1, v2);
    //   }
    // } break;

    // This makes the compiler happy.
    default:
      break;
  }
}

void PhaseScreen::loop() {
  // Update capture enabled if needed.
  // const bool run_button_checked =
  //     lv_obj_get_state(run_button_.lv_button, LV_BTN_PART_MAIN) &
  //     LV_STATE_CHECKED;
  // if (run_button_checked != capture_is_enabled_) {
  //   capture_is_enabled_ = run_button_checked;
  //   if (run_button_checked) {
  //     // Capture was just enabled.
  //     startCapture();
  //     set_displayed_status(RUNNING);
  //   } else {
  //     // Capture was just disabled. Nothing do do, we
  //     capture_in_progress_ = false;
  //     set_displayed_status(STOPPED);
  //   }
  //   return;
  // }

  // Nothing to do if capture is not enabled.
  // if (!capture_is_enabled_) {
  //   return;
  // }

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

  // TODO: Make class member? Share with other screen?
  static lv_point_t points[acquisition::kCaptureBufferSize];

  // Update both chart series with the captured data. We write
  // from scratch all chart's points.
  for (int i = 0; i < acquisition::kCaptureBufferSize; i++) {
    const acquisition::CaptureItem* item = items->get(i);
    // Currents in millamps [-2000, 2000].
    const int milliamps1 = acquisition::adc_value_to_milliamps(item->v1);
    const int milliamps2 = acquisition::adc_value_to_milliamps(item->v2);

    constexpr uint16_t x0 = 250;
    constexpr uint16_t y0 = 145;
    points[i].x = milliamps1 / 15 + x0;
    points[i].y = milliamps2 / 15 + y0;
    // lv_chart_set_point_id(chart_.lv_chart, chart_.ser1.lv_series, milliamps1,
    //                      i);
    // lv_chart_set_point_id(chart_.lv_chart, chart_.ser2.lv_series, milliamps2,
    //                      i);
  }

  lv_line_set_points(polar_chart_.lv_line, points,
                     acquisition::kCaptureBufferSize); /*Set the points*/

  // Chart is dirty. Mark it for refresh.
  // lv_chart_refresh(chart_.lv_chart);

  // Reset the display time, This will let the new display stay
  // on screen for a short time before we start a new capture.
  display_update_elapsed_.reset();
}
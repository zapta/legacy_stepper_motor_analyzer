#include "osciloscope_screen.h"

#include "analyzer/acquisition.h"
//#include "shared_capture_state.h"
#include "ui.h"

// TODO: clean code.


static constexpr capture_util::SharedCaptureState* shared_state = &capture_util::SharedCaptureState::instance;

// Convinience pointer.
//static constexpr capture_util::SharedCaptureState* shared_state_ = &capture_util::SharedCaptureState::instance;

// static constexpr uint32_t kUpdateIntervalMillis = 500;

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

// void OsciloscopeScreen::set_displayed_status(bool is_running) {
//   if (is_running) {
//     status_label_.set_text("RUNNING");
//     status_label_.set_text_color(LV_COLOR_GREEN);
//     run_button_.label.set_text(ui::kSymbolPause);
//     return;
//   }

//   // case STOPPED:
//   status_label_.set_text("STOPPED");
//   status_label_.set_text_color(LV_COLOR_RED);
//   run_button_.label.set_text(ui::kSymbolRun);
//   //     break;

//   //   default:
//   //     status_label_.set_text("???");
//   // }
// }

// Should work even if capture is in progress.
// void OsciloscopeScreen::startCapture() {
//   acquisition::start_capture(shared_state_->alternative_scale
//                                  ? kCaptureDividerAlternative
//                                  : kCaptureDividerNormal);
//   capture_in_progress_ = true;
// }

void OsciloscopeScreen::setup(uint8_t screen_num) {
  ui::create_screen(&screen_);
  ui::create_page_elements(screen_, "CURRENT PATTERNS", screen_num, nullptr);
  ui::create_chart(screen_, acquisition::kCaptureBufferSize, 2,
                   kAxisConfigsNormal, ui_events::UI_EVENT_SCALE, &chart_);
  capture_controls_.setup(screen_);
  // ui::create_label(screen_, 100, 130, 0, "", ui::kFontPageTitles,
  //                  LV_LABEL_ALIGN_CENTER, LV_COLOR_RED, &status_label_);

  // // Button's symbol is set later by set_displayed_status().
  // ui::create_button(screen_, 45, 150, ui::kBottomButtonsPosY, "", LV_COLOR_GRAY,
  //                   ui_events::UI_EVENT_CAPTURE, &run_button_);

  // lv_btn_set_checkable(run_button_.lv_button, true);

  // lv_obj_set_state(run_button_.lv_button, LV_STATE_CHECKED);
  // // set_displayed_status(true);
};

void OsciloscopeScreen::on_load() {
  // Update button.
  // if (shared_state_->is_capture_enabled) {
  //  lv_obj_set_state(run_button_.lv_button, LV_STATE_CHECKED);
  //} else {
  //  lv_obj_clear_state(run_button_.lv_button, LV_STATE_CHECKED);
  //}
  // Update
  update_display();

  // clear_chart();
  // Force display update on first loop.
  // lv_state_t old_state = lv_obj_get_state(run_button_.lv_button,
  // LV_BTN_PART_MAIN); lv_obj_set_state(run_button_.lv_button, old_state &
  // !LV_STATE_CHECKED);
  // //startCapture();

  // chart_.ser1.clear();
  // chart_.ser2.clear();
};

// void OsciloscopeScreen::clear_chart() {
//   chart_.ser1.clear();
//   chart_.ser2.clear();
//   // This enables capture.
//   lv_obj_set_state(run_button_.lv_button, LV_STATE_CHECKED);
//   lv_chart_refresh(chart_.lv_chart);
// }

void OsciloscopeScreen::on_event(ui_events::UiEventId ui_event_id) {
  switch (ui_event_id) {
    case ui_events::UI_EVENT_RESET:
      shared_state->clear_data();
      update_display();
      // clear_chart();
      break;

    case ui_events::UI_EVENT_SCALE: {
      shared_state->toggle_scale();
      update_display();
      // shared_state_->is_alternative_scale =
      //     !shared_state_->is_alternative_scale;
      // alternative_scale_ = !alternative_scale_;
      // if (scale_alternative_) {
      // chart_.set_scale(shared_state_->is_alternative_scale
      //                       ? kAxisConfigsAlternative
      //                      : kAxisConfigsNormal);
      //  clear_chart();
      // Serial.printf("Scale event %d\n",
      //              SharedCaptureState::instance.capture_alternative_scale);
    } break;

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

// Update chart from shared state.
void OsciloscopeScreen::update_display() {
  // TODO: can we skip this most of the times? Is it expensive?
  chart_.set_scale(shared_state->alternative_scale() ? kAxisConfigsAlternative
                                                       : kAxisConfigsNormal);

  capture_controls_.update(*shared_state);


  // No capture data.
  if (!shared_state->has_data()) {
    chart_.ser1.clear();
    chart_.ser2.clear();
    lv_chart_refresh(chart_.lv_chart);
    return;
  }

  // Has capture data.
  const acquisition::CaptureBuffer* capture_buffer =  shared_state->capture_buffer();
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

//}

void OsciloscopeScreen::loop() {
  // Update capture enabled if needed.
  if ( capture_controls_.maybe_update_state_from_button(shared_state)) {
    update_display();
    return;
  }

  if (shared_state->maybe_update_capture_data()) {
     update_display();
   }

  // const bool run_button_checked =
  //     lv_obj_get_state(capture_controls_.run_button.lv_button, LV_BTN_PART_MAIN) &
  //     LV_STATE_CHECKED;
  // if (run_button_checked != shared_state->capture_enabled()) {
  //   shared_state->set_capture_enabled(run_button_checked);
  //   update_display();
  //   // if (run_button_checked) {
  //   //   // Capture was just enabled.
  //   //   startCapture();
  //   //   update_display();
  //   //   //set_displayed_status(true);
  //   // } else {
  //   //   // Capture was just disabled. Nothing do do, we
  //   //   capture_in_progress_ = false;
  //   //   update_display();
  //   //   //set_displayed_status(false);
  //   // }
  //   return;
  // }

  // if (shared_state->maybe_update_capture_data()) {
  //   update_display();
  // }

  // // Nothing to do if capture is not enabled.
  // if (!SharedCaptureState::instance.capture_enabled) {
  //   return;
  // }

  // // Capture is enabled but capture is not in progress.
  // if (!capture_in_progress_) {
  //   if (display_update_elapsed_.elapsed_millis() >= kUpdateIntervalMillis) {
  //     startCapture();
  //   }
  //   return;
  // }

  // // Capture is enabled and is progress. Return if
  // // capture resutls are not ready yet.
  // if (!acquisition::is_capture_ready()) {
  //   return;
  // }

  // // Here where capture is enablled and the last
  // // capture request was just completed.
  // capture_in_progress_ = false;

  // // Since capture is not in progress now, the content of the
  // // capture buffer should be stable and it's safe to use
  // // it.
  // const acquisition::CaptureBuffer* capture_buffer =
  //     acquisition::capture_buffer();
  // // const acquisition::CaptureItems* items = &capture_buffer->items;

  // // Currently we display only captured with a trigger point.
  // // This seens to give a better user experience.
  // if (!capture_buffer->trigger_found) {
  //   return;
  // }

  // // Here we are comitted to display the new capture.
  // shared_state_->capture_buffer = *capture_buffer;
  // shared_state_->has_data = true;
  // // SharedCaptureState::instance.data_alternative_scale =
  // //    SharedCaptureState::instance.capture_alternative_scale;

  // update_display();

  // // Update both chart series with the captured data. We write
  // // // from scratch all chart's points.
  // // for (int i = 0; i < acquisition::kCaptureBufferSize; i++) {
  // //   const acquisition::CaptureItem* item =
  // //       SharedCaptureState::instance.capture_buffer.items.get(i);
  // //   // Currents in millamps [-2000, 2000].
  // //   const int milliamps1 = acquisition::adc_value_to_milliamps(item->v1);
  // //   const int milliamps2 = acquisition::adc_value_to_milliamps(item->v2);

  // //   lv_chart_set_point_id(chart_.lv_chart, chart_.ser1.lv_series, milliamps1,
  // //                         i);
  // //   lv_chart_set_point_id(chart_.lv_chart, chart_.ser2.lv_series, milliamps2,
  // //                         i);
  // // }

  // // // Chart is dirty. Mark it for refresh.
  // // lv_chart_refresh(chart_.lv_chart);

  // // Reset the display time, This will let the new display stay
  // // on screen for a short time before we start a new capture.
  // display_update_elapsed_.reset();
}
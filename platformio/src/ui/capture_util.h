// Allows to share capture data between screen.

// TODO: clean code.

#pragma once

#include "analyzer/acquisition.h"
#include "misc/elapsed.h"
#include "ui.h"

namespace capture_util {

class SharedCaptureState {
 public:
  void toggle_scale() {
    alternative_scale_ = !alternative_scale_;
    has_data_ = false;
    capture_enabled_ = true;
  }

  void clear_data() {
    has_data_ = false;
    capture_enabled_ = true;
  }

  void on_screen_load() {}

  bool maybe_update_capture_data();




  // True -> capture is running. False -> capture is stopped.

  // If has data is true, indicates the data scale of the data.
  // bool data_alternative_scale = false;
  // // If capture is enabled, indicates the time scale of the
  // // capture.

  // The only instance.
  static SharedCaptureState instance;

  inline bool alternative_scale() const { return alternative_scale_; }

  inline const acquisition::CaptureBuffer* capture_buffer() const {
    return &capture_buffer_;
  }

  inline bool capture_enabled() const { return capture_enabled_; }

  inline void set_capture_enabled(bool val) { capture_enabled_ = val; }

  inline bool has_data() { return has_data_; }

 private:
  SharedCaptureState() {}

  // If true, capture_buffer has valid capture data.
  bool has_data_ = false;

  bool capture_in_progress_ = false;

  bool alternative_scale_ = false;

  // Ignored in has_data is false.
  acquisition::CaptureBuffer capture_buffer_;

  bool capture_enabled_ = true;

    Elapsed elapsed_from_last_update_;

};

struct CaptureControls {
  ui::Button run_button;
  ui::Label status_label;

  void setup(ui::Screen& screen) {
    ui::create_label(screen, 100, 130, 0, "", ui::kFontPageTitles,
                     LV_LABEL_ALIGN_CENTER, LV_COLOR_RED, &status_label);

    // Button's symbol is set later by set_displayed_status().
    ui::create_button(screen, 45, 150, ui::kBottomButtonsPosY, "",
                      LV_COLOR_GRAY, ui_events::UI_EVENT_CAPTURE, &run_button);

    lv_btn_set_checkable(run_button.lv_button, true);

    lv_obj_set_state(run_button.lv_button, LV_STATE_CHECKED);
  }

  void update(const SharedCaptureState& shared_state) {
    // Update button
    if (shared_state.capture_enabled()) {
      lv_obj_set_state(run_button.lv_button, LV_STATE_CHECKED);
    } else {
      lv_obj_clear_state(run_button.lv_button, LV_STATE_CHECKED);
    }

    // Update status label.
    if (shared_state.capture_enabled()) {
      status_label.set_text("RUNNING");
      status_label.set_text_color(LV_COLOR_GREEN);
      run_button.label.set_text(ui::kSymbolPause);
    } else {
      status_label.set_text("STOPPED");
      status_label.set_text_color(LV_COLOR_RED);
      run_button.label.set_text(ui::kSymbolRun);
    }
  }

  bool maybe_update_state_from_button(SharedCaptureState* shared_state) {
    const bool run_button_checked =
        lv_obj_get_state(run_button.lv_button, LV_BTN_PART_MAIN) &
        LV_STATE_CHECKED;
    if (run_button_checked == shared_state->capture_enabled()) {
      return false;
    }

    shared_state->set_capture_enabled(run_button_checked);
    // update_display();
    // if (run_button_checked) {
    //   // Capture was just enabled.
    //   startCapture();
    //   update_display();
    //   //set_displayed_status(true);
    // } else {
    //   // Capture was just disabled. Nothing do do, we
    //   capture_in_progress_ = false;
    //   update_display();
    //   //set_displayed_status(false);
    // }
    return true;
  }
};

}  // namespace capture_util

#pragma once

#include "misc/elapsed.h"
#include "screen_manager.h"
#include "capture_util.h"

class PhaseScreen : public screen_manager::Screen {
 public:
  PhaseScreen() {};
  virtual void setup(uint8_t screen_num) override;
  virtual void on_load() override;
  virtual void loop() override;
  virtual void on_event(ui_events::UiEventId ui_event_id) override;

 private:
 enum Status {
  RUNNING,
  STOPPED
};

  // @@@@@ Delete these two
  void startCapture();
  //void set_displayed_status(Status status);
  void clear_chart();



  void update_display();

  //Elapsed display_update_elapsed_;
  ui::PolarChart polar_chart_;
  //bool capture_is_enabled_ = true;
  //bool capture_in_progress_ = false;
  //ui::Button run_button_;
  //ui::Label status_label_;
  //bool alternative_scale_ = false;

  //static constexpr capture_util::SharedCaptureState& shared_state_ = capture_util::SharedCaptureState::instance;

    capture_util::CaptureControls capture_controls_;


};
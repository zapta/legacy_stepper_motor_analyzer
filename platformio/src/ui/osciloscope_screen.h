#pragma once

#include "misc/elapsed.h"
#include "screen_manager.h"
#include "capture_util.h"

class OsciloscopeScreen : public screen_manager::Screen {
 public:
  OsciloscopeScreen() {};
  virtual void setup(uint8_t screen_num) override;
  virtual void on_load() override;
  virtual void loop() override;
  virtual void on_event(ui_events::UiEventId ui_event_id) override;

 private:
//  enum Status {
//   RUNNING,
//   STOPPED
// };

  //void startCapture();
  //void set_displayed_status(bool is_running);
  //void clear_chart();
  void update_display();

  //Elapsed display_update_elapsed_;
  ui::Chart chart_;
  //bool capture_is_enabled_ = true;
  //bool capture_in_progress_ = false;
  //ui::Button run_button_;
  //ui::Label status_label_;

  capture_util::CaptureControls capture_controls_;
  //bool alternative_scale_ = false;


};
#pragma once

#include "capture_util.h"
#include "misc/elapsed.h"
#include "screen_manager.h"

class PhaseScreen : public screen_manager::Screen {
 public:
  PhaseScreen(){};
  virtual void setup(uint8_t screen_num) override;
  virtual void on_load() override;
  virtual void loop() override;
  virtual void on_event(ui_events::UiEventId ui_event_id) override;

 private:
  void update_display();

  ui::PolarChart polar_chart_;
  capture_util::CaptureControls capture_controls_;
  ui::Label scale_lable_;
};
#pragma once

#include "misc/elapsed.h"
#include "screen_manager.h"

class OsciloscopeScreen : public screen_manager::Screen {
 public:
  OsciloscopeScreen() {};
  virtual void setup(uint8_t screen_num) override;
  virtual void on_load() override;
  virtual void on_unload() override;
  virtual void loop() override;
  virtual void on_event(ui_events::UiEventId ui_event_id) override;

 private:
 enum Status {
  RUNNING,
  STOPPED
};

  void startCapture();
  void set_displayed_status(Status status);
  void clear_chart();

  Elapsed display_update_elapsed_;
  ui::Chart chart_;
  bool capture_is_enabled_ = true;
  bool capture_in_progress_ = false;
  ui::Button run_button_;
  ui::Label status_label_;
  bool alternative_scale_ = false;
};
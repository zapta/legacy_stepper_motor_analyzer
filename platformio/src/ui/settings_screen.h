#pragma once

#include "misc/elapsed.h"
#include "screen_manager.h"

class SettingsScreen : public screen_manager::Screen {
 public:
  SettingsScreen();
  virtual void setup(uint8_t screen_num) override;
  virtual void on_load() override;
  virtual void on_unload() override;
  virtual void loop() override;
  virtual void on_event(ui_events::UiEventId ui_event_id) override;

 private:
  Elapsed display_update_elapsed_;
  ui::Label ch_a_field_;
  ui::Label ch_b_field_;
  ui::Checkbox reverse_checkbox_;
};
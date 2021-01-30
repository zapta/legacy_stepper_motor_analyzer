#pragma once

#include "misc/circular_buffer.h"
#include "misc/elapsed.h"
#include "screen_manager.h"

class RetractionChartScreen : public screen_manager::Screen {
 public:
  RetractionChartScreen(){};
  virtual void setup(uint8_t screen_num) override;
  virtual void on_load() override;
  virtual void on_unload() override;
  virtual void loop() override;
  virtual void on_event(ui_events::UiEventId ui_event_id) override;

 private:
  void clear_chart();

  static constexpr int16_t kNumPoints = 100;
  Elapsed display_update_elapsed_;
  // Counter to update the steps field once every N chart updates.
  uint8_t field_update_divider_ = 0;
  ui::Label retraction_field_;
  ui::Chart chart_;
  bool alternative_scale_ = false;
};
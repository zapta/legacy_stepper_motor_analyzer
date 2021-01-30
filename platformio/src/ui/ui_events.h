
#pragma once

#include "lvgl.h"

namespace ui_events {

enum UiEventId {
  UI_EVENT_NONE,
  UI_EVENT_RESET,
  UI_EVENT_NEXT_PAGE,
  UI_EVENT_PREV_PAGE,
  UI_EVENT_HOME_PAGE,
  UI_EVENT_SETTINGS,
  UI_EVENT_CAPTURE,
  UI_EVENT_ZERO_CALIBRATION,
  UI_EVENT_DIRECTION,
  UI_EVENT_SCALE,
  UI_EVENT_DEBUG,
  UI_EVENT_SCREENSHOT,
};

// Returns true and sets *ui_event_id if an event is pending.
extern bool consume_event(UiEventId* ui_event_id);
extern void clear_pending_events();

// Returns null if unknown id.
extern lv_event_cb_t get_event_handler(UiEventId ui_event_id);

}  // namespace ui_events

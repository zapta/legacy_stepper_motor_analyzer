#include "ui_events.h"

#include <Arduino.h>

#include "lvgl.h"

namespace ui_events {

static bool pending_event = false;
static UiEventId pending_event_id = UI_EVENT_NONE;

bool consume_event(UiEventId* ui_event_id) {
  if (!pending_event) {
    return false;
  }
  *ui_event_id = pending_event_id;
  pending_event = false;
  return true;
}

void clear_pending_events() { pending_event = false; }

static void common_event_handler(lv_obj_t* obj, lv_event_t event,
                                UiEventId ui_event_id) {
  if (event == LV_EVENT_CLICKED) {
    // Overwrite pending event, if exist.
    // TODO: should we have an event queue?
    pending_event = true;
    pending_event_id = ui_event_id;
  }
}

static void event_handler_reset(lv_obj_t* obj, lv_event_t event) {
  common_event_handler(obj, event, UI_EVENT_RESET);
}

static void event_handler_prev_page(lv_obj_t* obj, lv_event_t event) {
  common_event_handler(obj, event, UI_EVENT_PREV_PAGE);
}

static void event_handler_home_page(lv_obj_t* obj, lv_event_t event) {
  common_event_handler(obj, event, UI_EVENT_HOME_PAGE);
}

static void event_handler_next_page(lv_obj_t* obj, lv_event_t event) {
  common_event_handler(obj, event, UI_EVENT_NEXT_PAGE);
}

static void event_handler_settings(lv_obj_t* obj, lv_event_t event) {
  common_event_handler(obj, event, UI_EVENT_SETTINGS);
}

static void event_handler_capture(lv_obj_t* obj, lv_event_t event) {
  common_event_handler(obj, event, UI_EVENT_CAPTURE);
}

static void event_handler_zero_calibration(lv_obj_t* obj, lv_event_t event) {
  common_event_handler(obj, event, UI_EVENT_ZERO_CALIBRATION);
}

static void event_handler_direction(lv_obj_t* obj, lv_event_t event) {
  common_event_handler(obj, event, UI_EVENT_DIRECTION);
}

static void event_handler_scale(lv_obj_t* obj, lv_event_t event) {
  common_event_handler(obj, event, UI_EVENT_SCALE);
}

static void event_handler_debug(lv_obj_t* obj, lv_event_t event) {
  common_event_handler(obj, event, UI_EVENT_DEBUG);
}

static void event_handler_screenshot(lv_obj_t* obj, lv_event_t event) {
  common_event_handler(obj, event, UI_EVENT_SCREENSHOT);
}

// TODO: can we eliminate the need for individual callback functions
// and register the event type with LCGL?
//
extern lv_event_cb_t get_event_handler(UiEventId ui_event_id) {
  // NOTE: Event NONE is intentionally omitted.
  switch (ui_event_id) {
    case UI_EVENT_RESET:
      return event_handler_reset;
    case UI_EVENT_PREV_PAGE:
      return event_handler_prev_page;
    case UI_EVENT_HOME_PAGE:
      return event_handler_home_page;
    case UI_EVENT_NEXT_PAGE:
      return event_handler_next_page;
    case UI_EVENT_SETTINGS:
      return event_handler_settings;
    case UI_EVENT_CAPTURE:
      return event_handler_capture;
    case UI_EVENT_ZERO_CALIBRATION:
      return event_handler_zero_calibration;
    case UI_EVENT_DIRECTION:
      return event_handler_direction;
    case UI_EVENT_SCALE:
      return event_handler_scale;
    case UI_EVENT_DEBUG:
      return event_handler_debug;
    case UI_EVENT_SCREENSHOT:
      return event_handler_screenshot;
    default:
      return nullptr;
  }
}

}  // namespace ui_events

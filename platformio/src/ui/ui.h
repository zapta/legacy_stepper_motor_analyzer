
#pragma once

#include "lvgl.h"
#include "ui_events.h"

namespace ui {

extern const lv_font_t* const kFontSmallText;
extern const lv_font_t* const kFontPageTitles;
extern const lv_font_t* const kFontDataFields;
extern const lv_font_t* const kFontNumericDataFields;
extern const lv_font_t* const kFontLargeNumericFields;
extern const lv_font_t* const kFontIconButtons;

constexpr int kBottomButtonsPosY = 285;
constexpr int kButtonHeight = 35;

// Constantns for the Symbols font.
constexpr const char* kSymbolRun = "I";
constexpr const char* kSymbolPause = "J";
constexpr const char* kSymbolHome = "A";
constexpr const char* kSymbolNext = "6";
constexpr const char* kSymbolPrev = "5";
constexpr const char* kSymbolOk = "0";
constexpr const char* kSymbolDelete = "B";
constexpr const char* kSymbolSettings = "3";

struct Screen {
  lv_obj_t* lv_screen = nullptr;
};

struct GaugeAxisConfig {
  int32_t min;
  int32_t max;
  uint8_t major_intervals;
  uint8_t minor_intervals;
};

struct Gauge {
  lv_obj_t* lv_gauge = nullptr;

  void set_scale(const GaugeAxisConfig& config);
};

struct Label {
  lv_obj_t* lv_label = nullptr;

  void set_text_int(int32_t i) { lv_label_set_text_fmt(lv_label, "%d", i); }

  void set_text_uint(uint32_t u) { lv_label_set_text_fmt(lv_label, "%u", u); }
  void set_text(const char* s) { lv_label_set_text(lv_label, s); }
  void set_text_float(double f, uint8_t precision);
  void set_text_color(lv_color_t text_color) {
    lv_obj_set_style_local_text_color(lv_label, LV_LABEL_PART_MAIN,
                                      LV_STATE_DEFAULT, text_color);
  }
  void set_click_event(ui_events::UiEventId ui_event_id) {
    if (ui_event_id != ui_events::UI_EVENT_NONE) {
      lv_obj_set_click(lv_label, true);
      const lv_event_cb_t event_cb = ui_events::get_event_handler(ui_event_id);
      lv_obj_set_event_cb(lv_label, event_cb);
    }
  }
};

struct Checkbox {
  lv_obj_t* lv_checkbox = nullptr;
  bool is_checked() { return lv_checkbox_is_checked(lv_checkbox); }
  void set_is_checked(bool b) { lv_checkbox_set_checked(lv_checkbox, b); }
};

struct Button {
  lv_obj_t* lv_button = nullptr;
  Label label;
};

struct Range {
  lv_coord_t min = 0;
  lv_coord_t max = 199;
};

struct ChartAxisConfig {
  const char* labels = nullptr;
  uint8_t num_ticks = 0;
  // Vertical or horizontal internal grid lines.
  uint8_t dividers;
  bool is_enabled() const { return num_ticks > 0 && labels != nullptr; }
};

struct ChartAxisConfigs {
  Range y_range;
  ChartAxisConfig x;
  ChartAxisConfig y;
};

struct ChartSeries {
  lv_obj_t* lv_chart = nullptr;
  lv_chart_series_t* lv_series = nullptr;

  void set_next(lv_coord_t v) {
    if (filled) {
      lv_chart_set_next(lv_chart, lv_series, v);
      return;
    }

    const uint16_t n = lv_chart_get_point_count(lv_chart);
    lv_chart_set_point_id(lv_chart, lv_series, v, fill_counter);
    lv_chart_refresh(lv_chart);
    if (++fill_counter >= n) {
      filled = true;
    }
  }

  void clear() {
    lv_chart_clear_series(lv_chart, lv_series);
    filled = false;
    fill_counter = 0;
  }

 private:
  bool filled = false;
  uint16_t fill_counter = 0;
};

struct Chart {
  lv_obj_t* lv_chart = nullptr;
  ChartSeries ser1;
  ChartSeries ser2;

  void set_scale(const ChartAxisConfigs& axis_configs);
};

struct Histogram {
  lv_obj_t* lv_chart = nullptr;
  lv_chart_series_t* lv_series = nullptr;
};

struct PolarChart {
  lv_coord_t max_radius;
  lv_obj_t* lv_chart = nullptr;
  lv_obj_t* lv_line = nullptr;
};

struct StdPageElements {
  Label screen_number;
  Label title;
  Button reset_button;
  Button prev_button;
  Button home_button;
  Button next_button;
};

extern void create_screen(Screen* screen);

extern void create_gauge(const Screen& screen, const GaugeAxisConfig& config,

                         ui_events::UiEventId ui_event_id, Gauge* gauge);

// If 'button' is null then it is ignored;
extern void create_button(const Screen& screen, lv_coord_t width, lv_coord_t x,
                          lv_coord_t y, const char* kFootnotText,
                          lv_color_t color, ui_events::UiEventId ui_event_id,
                          Button* button);

// If 'label' is null then it is ignored. width == 0 indicates auto size.
extern void create_label(const Screen& screen, lv_coord_t width, lv_coord_t x,
                         lv_coord_t y, const char* kFootnotText,
                         const lv_font_t* lv_font, lv_label_align_t label_align,
                         lv_color_t color, Label* label);

// If 'checkbox' is null then it is ignored.
extern void create_checkbox(const Screen& screen, lv_coord_t x, lv_coord_t y,
                            const char* kFootnotText, const lv_font_t* lv_font,
                            lv_color_t text_color,
                            ui_events::UiEventId ui_event_id,
                            Checkbox* checkbox);

extern void create_chart(const Screen& screen, uint16_t num_points,
                         int num_series, const ChartAxisConfigs& axis_configs,
                         ui_events::UiEventId ui_event_id, Chart* chart);

extern void create_histogram(const Screen& screen, uint16_t num_columns,
                             const ChartAxisConfigs& axis_configs,
                             Histogram* histogram);

// If 'lable' is null it is ignored.
void create_page_title(const Screen& screen, const char* title, Label* label);

void create_polar_chart(const Screen& screen,
                        const ChartAxisConfigs& axis_configs,
                        ui_events::UiEventId ui_event_id,
                        PolarChart* polar_chart);

// If 'page_elements' is null then it is ignored. Screen number is ignored if
// zero.
extern void create_page_elements(const Screen& screen, const char* title,
                                 uint8_t screen_number,
                                 StdPageElements* page_elements);

}  // namespace ui

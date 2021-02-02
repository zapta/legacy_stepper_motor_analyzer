#include "ui.h"

#include <Arduino.h>

#include "lvgl.h"

namespace ui {

// For UI debugging. Shows the boundaries of UI objects.
static constexpr bool kDebugBackgrounds = false;

// For developers only. When enabled, clicking on a
// screen's title field pauses the program and sends a screen
// dump over the USB/serial connection.
static constexpr bool kEnableScreenshots = false;

static const lv_color_t kDebugBackgroundColor = LV_COLOR_MAKE(0x40, 0x40, 0x40);

static void init_styles_if_needed();
static bool styles_initialized = false;

struct GaugeStyles {
  lv_style_t main;
  lv_style_t major;
  lv_style_t needle;
};

static GaugeStyles gauge_styles;

static const lv_color_t kGuageNeedleColor[] = {LV_COLOR_RED};

struct ButtonStyles {
  lv_style_t main;
};
static ButtonStyles button_style;

struct ChartStyles {
  lv_style_t bg;
  lv_style_t series;
  lv_style_t series_bg;
};
static ChartStyles chart_styles;

struct HistogramStyles {
  lv_style_t bg;
  lv_style_t series;
  lv_style_t series_bg;
};

static HistogramStyles histogram_styles;

static char temp_text_buffer[20];

const lv_font_t* const kFontSmallText = &font_montserrat_alphanum_12;
const lv_font_t* const kFontPageTitles = &font_montserrat_alphanum_16;
const lv_font_t* const kFontDataFields = &font_montserrat_alphanum_20;
const lv_font_t* const kFontNumericDataFields = &font_montserrat_numeric_20;
const lv_font_t* const kFontLargeNumericFields = &font_montserrat_numeric_50;
const lv_font_t* const kFontIconButtons = &font_fa_symbols_28;

static void init_gauge_styles() {
  // Main style
  lv_style_init(&gauge_styles.main);
  lv_style_set_bg_color(&gauge_styles.main, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_style_set_text_color(&gauge_styles.main, LV_STATE_DEFAULT, LV_COLOR_GRAY);
  lv_style_set_border_width(&gauge_styles.main, LV_STATE_DEFAULT, 0);
  lv_style_set_line_width(&gauge_styles.main, LV_STATE_DEFAULT, 3);
  lv_style_set_line_color(&gauge_styles.main, LV_STATE_DEFAULT, LV_COLOR_GRAY);
  lv_style_set_scale_grad_color(&gauge_styles.main, LV_STATE_DEFAULT,
                                LV_COLOR_GRAY);

  // Major style.
  lv_style_init(&gauge_styles.major);
  lv_style_set_line_width(&gauge_styles.major, LV_STATE_DEFAULT, 5);
  lv_style_set_line_color(&gauge_styles.major, LV_STATE_DEFAULT, LV_COLOR_GRAY);
  lv_style_set_scale_grad_color(&gauge_styles.major, LV_STATE_DEFAULT,
                                LV_COLOR_GRAY);

  // Needle style.
  lv_style_init(&gauge_styles.needle);
  lv_style_set_line_width(&gauge_styles.needle, LV_STATE_DEFAULT, 8);
}

static void init_button_styles() {
  lv_style_init(&button_style.main);
  lv_style_set_radius(&button_style.main, LV_STATE_DEFAULT, 4);

  lv_style_set_bg_color(&button_style.main, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_style_set_bg_color(&button_style.main, LV_STATE_CHECKED, LV_COLOR_BLACK);

  // These provide a visual feedback when the buton is pressed.
  lv_style_set_bg_color(&button_style.main, LV_STATE_PRESSED, LV_COLOR_CYAN);
  lv_style_set_bg_color(&button_style.main, LV_STATE_PRESSED | LV_STATE_CHECKED,
                        LV_COLOR_CYAN);

  lv_style_set_text_font(&button_style.main, LV_STATE_DEFAULT,
                         kFontIconButtons);

  lv_style_set_border_width(&button_style.main, LV_STATE_DEFAULT, 0);
  lv_style_set_outline_width(&button_style.main, LV_STATE_DEFAULT, 0);
}

// Common to charts and histograms.
static void common_lv_chart_bg_style(lv_style_t* bg_style) {
  lv_style_set_radius(bg_style, LV_STATE_DEFAULT, 0);
  lv_style_set_bg_color(
      bg_style, LV_STATE_DEFAULT,
      kDebugBackgrounds ? kDebugBackgroundColor : LV_COLOR_BLACK);

  lv_style_set_text_font(bg_style, LV_STATE_DEFAULT,
                         &font_montserrat_alphanum_12);
  lv_style_set_text_color(bg_style, LV_STATE_DEFAULT, LV_COLOR_GRAY);

  lv_style_set_border_width(bg_style, LV_STATE_DEFAULT, 0);
  lv_style_set_outline_width(bg_style, LV_STATE_DEFAULT, 0);
}

static void init_chart_styles() {
  // Background style
  lv_style_init(&chart_styles.bg);

  common_lv_chart_bg_style((&chart_styles.bg));

  // Series style
  lv_style_init(&chart_styles.series);
  lv_style_set_size(&chart_styles.series, LV_STATE_DEFAULT, 0);
  lv_style_set_line_width(&chart_styles.series, LV_STATE_DEFAULT, 2);

  // Series bg style
  lv_style_init(&chart_styles.series_bg);
  lv_style_set_line_dash_gap(&chart_styles.series_bg, LV_STATE_DEFAULT, 0);
  lv_style_set_line_color(&chart_styles.series_bg, LV_STATE_DEFAULT,
                          LV_COLOR_MAKE(0x00, 0x40, 0x00));  // dark green
}

static void init_histogram_styles() {
  // Background style
  lv_style_init(&histogram_styles.bg);

  common_lv_chart_bg_style((&histogram_styles.bg));

  // Series style
  lv_style_init(&histogram_styles.series);
  // Disable bars' corner rounding.
  lv_style_set_radius(&histogram_styles.series, LV_STATE_DEFAULT, 0);

  // Series bg style
  lv_style_init(&histogram_styles.series_bg);
  // lv_style_set_line_width(&histogram_styles.series, LV_STATE_DEFAULT, 0);
  lv_style_set_line_dash_gap(&histogram_styles.series_bg, LV_STATE_DEFAULT, 0);

  lv_style_set_line_color(&histogram_styles.series_bg, LV_STATE_DEFAULT,
                          LV_COLOR_MAKE(0x00, 0x40, 0x00));  // dark green
}

void create_screen(Screen* screen) {
  lv_obj_t* lv_screen = lv_obj_create(NULL, NULL);
  lv_obj_set_style_local_bg_color(lv_screen, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,
                                  LV_COLOR_BLACK);
  lv_obj_set_click(lv_screen, false);
  screen->lv_screen = lv_screen;
}

static void set_gauge_scale(lv_obj_t* lv_gauge, const GaugeAxisConfig& config) {
  lv_gauge_set_scale(lv_gauge, 290,
                     (config.minor_intervals) * (config.major_intervals) + 1,
                     config.major_intervals + 1);

  lv_gauge_set_range(lv_gauge, config.min, config.max);

  lv_gauge_set_critical_value(lv_gauge, config.max + 1);

  lv_obj_invalidate(lv_gauge);
}

void Gauge::set_scale(const GaugeAxisConfig& config) {
  set_gauge_scale(lv_gauge, config);
}

void create_gauge(const Screen& screen, const GaugeAxisConfig& config,
                  ui_events::UiEventId ui_event_id,

                  Gauge* gauge) {
  init_styles_if_needed();

  lv_obj_t* lv_gauge = lv_gauge_create(screen.lv_screen, NULL);
  lv_obj_add_style(lv_gauge, LV_GAUGE_PART_MAIN, &gauge_styles.main);
  lv_obj_add_style(lv_gauge, LV_GAUGE_PART_MAJOR, &gauge_styles.major);
  lv_obj_add_style(lv_gauge, LV_GAUGE_PART_NEEDLE, &gauge_styles.needle);

  lv_obj_set_click(lv_gauge, false);

  // Workaround: This hids the 'critical' range by making it looks like the
  // normal range. As of Dec 2020, there is no programatic way to disable the
  // 'end' (aka critical) range.
  //
  // https://forum.lvgl.io/t/how-to-disable-the-critical-color-of-lv-gauge/4199
  lv_obj_set_style_local_scale_end_color(lv_gauge, LV_GAUGE_PART_MAJOR,
                                         LV_STATE_DEFAULT, LV_COLOR_GRAY);
  lv_obj_set_style_local_scale_end_line_width(lv_gauge, LV_GAUGE_PART_MAJOR,
                                              LV_STATE_DEFAULT, 5);

  lv_gauge_set_needle_count(lv_gauge, 1, kGuageNeedleColor);

  lv_obj_set_size(lv_gauge, 270, 270);
  lv_obj_set_pos(lv_gauge, 0, 20);

  set_gauge_scale(lv_gauge, config);

  if (ui_event_id != ui_events::UI_EVENT_NONE) {
    lv_obj_set_click(lv_gauge, true);

    // Nullable.
    const lv_event_cb_t event_cb = ui_events::get_event_handler(ui_event_id);
    // Nulls are trapped.
    lv_obj_set_event_cb(lv_gauge, event_cb);
  }

  if (kDebugBackgrounds) {
    lv_obj_set_style_local_bg_opa(lv_gauge, LV_GAUGE_PART_MAIN,
                                  LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_obj_set_style_local_bg_color(lv_gauge, LV_GAUGE_PART_MAIN,
                                    LV_STATE_DEFAULT, kDebugBackgroundColor);
  }

  gauge->lv_gauge = lv_gauge;
}

void create_button(const Screen& screen, lv_coord_t width, lv_coord_t x,
                   lv_coord_t y, const char* kFootnotText, lv_color_t color,
                   ui_events::UiEventId ui_event_id, Button* button) {
  init_styles_if_needed();

  lv_obj_t* lv_button = lv_btn_create(screen.lv_screen, NULL);
  lv_obj_set_size(lv_button, width, kButtonHeight);

  lv_obj_t* lv_label = lv_label_create(lv_button, NULL);
  lv_label_set_text(lv_label, kFootnotText);
  lv_label_set_align(lv_label, LV_ALIGN_CENTER);

  lv_obj_set_style_local_text_color(lv_label, LV_OBJ_PART_MAIN,
                                    LV_STATE_DEFAULT, color);

  lv_obj_add_style(lv_button, LV_OBJ_PART_MAIN, &button_style.main);

  lv_obj_set_pos(lv_button, x, y);

  if (ui_event_id != ui_events::UI_EVENT_NONE) {
    // Nullable.
    const lv_event_cb_t event_cb = ui_events::get_event_handler(ui_event_id);

    lv_obj_set_event_cb(lv_button, event_cb);
  }

  if (kDebugBackgrounds) {
    lv_obj_set_style_local_bg_opa(lv_button, LV_BTN_PART_MAIN, LV_STATE_DEFAULT,
                                  LV_OPA_COVER);
    lv_obj_set_style_local_bg_color(lv_button, LV_BTN_PART_MAIN,
                                    LV_STATE_DEFAULT, kDebugBackgroundColor);
  }

  if (button != nullptr) {
    button->lv_button = lv_button;
    button->label.lv_label = lv_label;
  }
}

// If 'label' is nullptr, it is ignored.
// width == 0 indicates auto size.
void create_label(const Screen& screen, lv_coord_t width, lv_coord_t x,
                  lv_coord_t y, const char* kFootnotText,
                  const lv_font_t* lv_font, lv_label_align_t label_align,
                  lv_color_t text_color, Label* label) {
  init_styles_if_needed();
  lv_obj_t* lv_label = lv_label_create(screen.lv_screen, NULL);
  lv_label_set_text(lv_label, kFootnotText);
  lv_obj_set_style_local_text_font(lv_label, LV_LABEL_PART_MAIN,
                                   LV_STATE_DEFAULT, lv_font);
  lv_label_set_align(lv_label, label_align);
  lv_label_set_long_mode(lv_label, LV_LABEL_LONG_CROP);
  if (width != 0) {
    lv_obj_set_size(lv_label, width, lv_font->line_height);
  }
  lv_obj_set_pos(lv_label, x, y);
  lv_obj_set_style_local_text_color(lv_label, LV_LABEL_PART_MAIN,
                                    LV_STATE_DEFAULT, text_color);
  lv_obj_set_click(lv_label, false);

  if (kDebugBackgrounds) {
    lv_obj_set_style_local_bg_opa(lv_label, LV_LABEL_PART_MAIN,
                                  LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_obj_set_style_local_bg_color(lv_label, LV_LABEL_PART_MAIN,
                                    LV_STATE_DEFAULT, kDebugBackgroundColor);
  }

  if (label != nullptr) {
    label->lv_label = lv_label;
  }
}

void create_checkbox(const Screen& screen, lv_coord_t x, lv_coord_t y,
                     const char* text, const lv_font_t* lv_font,
                     lv_color_t text_color, ui_events::UiEventId ui_event_id,
                     Checkbox* checkbox) {
  lv_obj_t* lv_checkbox = lv_checkbox_create(screen.lv_screen, NULL);
  lv_obj_set_pos(lv_checkbox, x, y);
  lv_checkbox_set_text(lv_checkbox, text);

  lv_obj_set_style_local_text_font(lv_checkbox, LV_LABEL_PART_MAIN,
                                   LV_STATE_DEFAULT, lv_font);

  lv_obj_set_style_local_text_color(lv_checkbox, LV_LABEL_PART_MAIN,
                                    LV_STATE_DEFAULT, text_color);

  lv_obj_set_style_local_outline_width(lv_checkbox, LV_STATE_DEFAULT,
                                       LV_STATE_DEFAULT, 0);

  lv_obj_set_style_local_bg_color(lv_checkbox, LV_CHECKBOX_PART_BULLET,
                                  LV_STATE_DEFAULT, LV_COLOR_GRAY);

  lv_obj_set_style_local_pattern_recolor(lv_checkbox, LV_CHECKBOX_PART_BULLET,
                                         LV_STATE_CHECKED, LV_COLOR_BLACK);

  const lv_event_cb_t event_cb = ui_events::get_event_handler(ui_event_id);
  // if (event_cb != nullptr) {
  // Nulls are traped.
  lv_obj_set_event_cb(lv_checkbox, event_cb);

  if (checkbox != nullptr) {
    checkbox->lv_checkbox = lv_checkbox;
  }
}

void set_chart_scale(lv_obj_t* lv_chart, const ChartAxisConfigs& axis_configs) {
  // Number of internal vertical and horizontal grid lines. Does not
  // include the frame.
  lv_chart_set_div_line_count(lv_chart, axis_configs.y.dividers,
                              axis_configs.x.dividers);

  lv_chart_set_y_tick_length(lv_chart, 0, 0);
  lv_chart_set_x_tick_length(lv_chart, 0, 0);

  if (axis_configs.x.is_enabled()) {
    lv_chart_set_x_tick_texts(lv_chart, axis_configs.x.labels,
                              axis_configs.x.num_ticks,
                              LV_CHART_AXIS_DRAW_LAST_TICK);
  }

  if (axis_configs.y.is_enabled()) {
    lv_chart_set_y_tick_texts(lv_chart, axis_configs.y.labels,
                              axis_configs.y.num_ticks,
                              LV_CHART_AXIS_DRAW_LAST_TICK);
  }

  lv_chart_set_y_range(lv_chart, LV_CHART_AXIS_PRIMARY_Y,
                       axis_configs.y_range.min, axis_configs.y_range.max);
}

void Chart::set_scale(const ChartAxisConfigs& axis_configs) {
  set_chart_scale(lv_chart, axis_configs);
  lv_chart_refresh(lv_chart);
}

// Common to charts and histograms.
static void common_lv_chart_settings(lv_obj_t* lv_chart,
                                     const ChartAxisConfigs& axis_configs) {
  lv_obj_set_click(lv_chart, false);

  // NOTE: Padding on top and right is required to avoid clipping the
  // x and y lables.
  const bool y_labels_enabled = axis_configs.y.is_enabled();
  lv_obj_set_style_local_pad_top(lv_chart, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,
                                 10);
  lv_obj_set_style_local_pad_bottom(lv_chart, LV_OBJ_PART_MAIN,
                                    LV_STATE_DEFAULT, 40);
  lv_obj_set_style_local_pad_right(lv_chart, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,
                                   20);
  lv_obj_set_style_local_pad_left(lv_chart, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,
                                  y_labels_enabled ? 40 : 20);

  lv_obj_set_size(lv_chart, 460, 240);
  // The x,y offsets here are fine tweaks of the chat position.
  lv_obj_align(lv_chart, NULL, LV_ALIGN_CENTER, 0, 0);

  set_chart_scale(lv_chart, axis_configs);
}

void create_chart(const Screen& screen, uint16_t num_points, int num_series,
                  const ChartAxisConfigs& axis_configs,
                  ui_events::UiEventId ui_event_id, Chart* chart) {
  init_styles_if_needed();

  lv_obj_t* lv_chart = lv_chart_create(screen.lv_screen, NULL);
  common_lv_chart_settings(lv_chart, axis_configs);
  lv_chart_set_type(lv_chart, LV_CHART_TYPE_LINE);
  lv_chart_set_update_mode(lv_chart, LV_CHART_UPDATE_MODE_SHIFT);
  // TODO: set this to actual number of X pixels for better resolution.
  lv_chart_set_point_count(lv_chart, num_points);

  if (ui_event_id != ui_events::UI_EVENT_NONE) {
    lv_obj_set_click(lv_chart, true);
    const lv_event_cb_t event_cb = ui_events::get_event_handler(ui_event_id);
    lv_obj_set_event_cb(lv_chart, event_cb);
  }

  // Add first data series.
  lv_chart_series_t* lv_series1 =
      lv_chart_add_series(lv_chart, LV_COLOR_YELLOW);
  // Maybe add a second data series.
  lv_chart_series_t* lv_series2 =
      (num_series >= 2) ? lv_chart_add_series(lv_chart, LV_COLOR_CYAN)
                        : nullptr;

  // Apply styles.
  lv_obj_add_style(lv_chart, LV_CHART_PART_BG,
                   &chart_styles.bg);  // apply background style
  lv_obj_add_style(lv_chart, LV_CHART_PART_SERIES,
                   &chart_styles.series);  // Apply part series style.
  lv_obj_add_style(lv_chart, LV_CHART_PART_SERIES_BG,
                   &chart_styles.series_bg);  // apply series background style

  chart->lv_chart = lv_chart;

  chart->ser1.lv_chart = lv_chart;
  chart->ser1.lv_series = lv_series1;

  chart->ser2.lv_chart = lv_chart;
  chart->ser2.lv_series = lv_series2;
}

void create_histogram(const Screen& screen, uint16_t num_columns,
                      const ChartAxisConfigs& axis_configs,
                      Histogram* histogram) {
  init_styles_if_needed();

  lv_obj_t* lv_chart = lv_chart_create(screen.lv_screen, NULL);

  common_lv_chart_settings(lv_chart, axis_configs);

  // lv_obj_align(lv_chart, NULL, LV_ALIGN_CENTER, 0, 0);
  lv_chart_set_type(lv_chart, LV_CHART_TYPE_COLUMN);

  lv_chart_set_point_count(lv_chart, num_columns);

  // Add a data series.
  lv_chart_series_t* lv_series = lv_chart_add_series(lv_chart, LV_COLOR_YELLOW);

  lv_chart_set_y_range(lv_chart, LV_CHART_AXIS_PRIMARY_Y,
                       axis_configs.y_range.min, axis_configs.y_range.max);

  lv_obj_add_style(lv_chart, LV_CHART_PART_BG,
                   &histogram_styles.bg);  // apply background style
  lv_obj_add_style(lv_chart, LV_CHART_PART_SERIES,
                   &histogram_styles.series);  // Apply part series style.
  lv_obj_add_style(
      lv_chart, LV_CHART_PART_SERIES_BG,
      &histogram_styles.series_bg);  // apply series background style

  histogram->lv_chart = lv_chart;
  histogram->lv_series = lv_series;
}

void create_page_title(const Screen& screen, const char* title, Label* label) {
  Label tmp_label;
  Label* title_label_ptr = (label == nullptr) ? &tmp_label : label;

  ui::create_label(screen, 220, 480 - 220 - 5, 0, title, kFontPageTitles,
                   LV_LABEL_ALIGN_RIGHT, LV_COLOR_GRAY, title_label_ptr);

  if (kEnableScreenshots) {
    lv_obj_set_click(title_label_ptr->lv_label, true);
    const lv_event_cb_t event_cb =
        ui_events::get_event_handler(ui_events::UI_EVENT_SCREENSHOT);
    lv_obj_set_event_cb(title_label_ptr->lv_label, event_cb);
  }
}

void create_page_elements(const Screen& screen, const char* title,
                          uint8_t screen_number,
                          StdPageElements* page_elements) {
  const bool is_null = page_elements == nullptr;
  ui::create_button(screen, 50, 0, kBottomButtonsPosY, kSymbolDelete,
                    LV_COLOR_GRAY, ui_events::UI_EVENT_RESET,
                    is_null ? nullptr : &page_elements->reset_button);

  ui::create_button(screen, 40, 300, kBottomButtonsPosY, kSymbolPrev,
                    LV_COLOR_GRAY, ui_events::UI_EVENT_PREV_PAGE,
                    is_null ? nullptr : &page_elements->prev_button);

  ui::create_button(screen, 50, 365, kBottomButtonsPosY, kSymbolHome,
                    LV_COLOR_GRAY, ui_events::UI_EVENT_HOME_PAGE,
                    is_null ? nullptr : &page_elements->home_button);

  ui::create_button(screen, 40, 480 - 40, kBottomButtonsPosY, kSymbolNext,
                    LV_COLOR_GRAY, ui_events::UI_EVENT_NEXT_PAGE,
                    is_null ? nullptr : &page_elements->next_button);

  Label tmp_label;
  Label* num_label_ptr = is_null ? &tmp_label : &page_elements->screen_number;
  ui::create_label(screen, 40, 5, 0, "", kFontPageTitles, LV_LABEL_ALIGN_LEFT,
                   LV_COLOR_GRAY, num_label_ptr);
  if (screen_number != 0) {
    num_label_ptr->set_text_int(screen_number);
  }

  create_page_title(screen, title, is_null ? nullptr : &page_elements->title);
}

static void init_styles_if_needed() {
  if (!styles_initialized) {
    init_gauge_styles();
    init_button_styles();
    init_chart_styles();
    init_histogram_styles();
    styles_initialized = true;
  }
}

void Label::set_text_float(double f, uint8_t precision) {
  dtostrf(f, 0, precision, temp_text_buffer);
  // TODO: find a conversion method that doesn't return negative
  // zeros. Until then, use the hack below.
  const char* value_str = strncmp("-0.", temp_text_buffer, 3) == 0
                              ? temp_text_buffer + 1
                              : temp_text_buffer;
  // This methods makes a copy of the string and doesn't maintain the input
  // pointer.
  lv_label_set_text(lv_label, value_str);
}

}  // namespace ui

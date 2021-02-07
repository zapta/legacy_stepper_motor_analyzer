#include "home_screen.h"

#include "analyzer/acquisition.h"
#include "misc/config_eeprom.h"
#include "ui.h"

static constexpr uint32_t kUpdateIntervalMillis = 500;

HomeScreen::HomeScreen(){};

void HomeScreen::setup(uint8_t screen_num) {
  ui::create_screen(&screen_);
  ui::create_page_elements(screen_, "HOME", screen_num, nullptr);
  ui::create_button(screen_, 50, 120, ui::kBottomButtonsPosY,
                    ui::kSymbolSettings, LV_COLOR_GRAY,
                    ui_events::UI_EVENT_SETTINGS, nullptr);

  // We don't bother to keep references to the fixed labels.
  const lv_coord_t w1 = 100;
  const lv_coord_t w2 = 130;
  const lv_coord_t x1 = 40;
  const lv_coord_t x2 = 150;
  const lv_coord_t dy = 36;
  lv_coord_t y = 47;

  // NOTE: adding 1 to the y of the numeric fields to better align
  // with the font of the text fields.
  ui::create_label(screen_, w1, x1, y, "COIL 1", ui::kFontDataFields,
                   LV_LABEL_ALIGN_LEFT, LV_COLOR_SILVER, nullptr);

  ui::create_label(screen_, w2, x2, y + 1, "", ui::kFontNumericDataFields,
                   LV_LABEL_ALIGN_RIGHT, LV_COLOR_SILVER, &ch_a_field_);
  y += dy;

  ui::create_label(screen_, w1, x1, y + 1, "COIL 2", ui::kFontDataFields,
                   LV_LABEL_ALIGN_LEFT, LV_COLOR_SILVER, nullptr);
  ui::create_label(screen_, w2, x2, y, "", ui::kFontNumericDataFields,
                   LV_LABEL_ALIGN_RIGHT, LV_COLOR_SILVER, &ch_b_field_);
  y += dy;

  ui::create_label(screen_, w1, x1, y, "ERRORS", ui::kFontDataFields,
                   LV_LABEL_ALIGN_LEFT, LV_COLOR_SILVER, nullptr);
  ui::create_label(screen_, w2, x2, y + 1, "", ui::kFontNumericDataFields,
                   LV_LABEL_ALIGN_RIGHT, LV_COLOR_SILVER, &errors_field_);
  y += dy;

  ui::create_label(screen_, w1, x1, y, "POWER", ui::kFontDataFields,
                   LV_LABEL_ALIGN_LEFT, LV_COLOR_SILVER, nullptr);
  ui::create_label(screen_, w2, x2, y + 1, "", ui::kFontDataFields,
                   LV_LABEL_ALIGN_RIGHT, LV_COLOR_SILVER, &power_field_);
  y += dy;

  ui::create_label(screen_, w1, x1, y, "IDLES", ui::kFontDataFields,
                   LV_LABEL_ALIGN_LEFT, LV_COLOR_SILVER, nullptr);
  ui::create_label(screen_, w2, x2, y + 1, "", ui::kFontNumericDataFields,
                   LV_LABEL_ALIGN_RIGHT, LV_COLOR_SILVER, &idles_field_);
  y += dy;

  ui::create_label(screen_, w1 - 20, x1, y, "STEPS", ui::kFontDataFields,
                   LV_LABEL_ALIGN_LEFT, LV_COLOR_YELLOW, nullptr);
  ui::create_label(screen_, w2 + 20, x2 - 20, y + 1, "",
                   ui::kFontNumericDataFields, LV_LABEL_ALIGN_RIGHT,
                   LV_COLOR_YELLOW, &steps_field_);
};

void HomeScreen::on_load() {
  // Force display update on first loop.
  display_update_elapsed_.set(kUpdateIntervalMillis + 1);
};

void HomeScreen::on_unload(){};

void HomeScreen::loop() {
  // We update at a fixed rate.
  if (display_update_elapsed_.elapsed_millis() < kUpdateIntervalMillis) {
    return;
  }
  display_update_elapsed_.advance(kUpdateIntervalMillis);

  // Sample data and update screen.
  const acquisition::State* state = acquisition::sample_state();

  ch_a_field_.set_text_float(acquisition::adc_value_to_amps(state->v1), 2);
  ch_b_field_.set_text_float(acquisition::adc_value_to_amps(state->v2), 2);

  errors_field_.set_text_uint(state->quadrature_errors);
  errors_field_.set_text_color(state->quadrature_errors ? LV_COLOR_RED
                                                        : LV_COLOR_SILVER);
  power_field_.set_text(state->is_energized ? "ON" : "OFF");
  power_field_.set_text_color(state->is_energized ? LV_COLOR_SILVER
                                                  : LV_COLOR_RED);
  idles_field_.set_text_uint(state->non_energized_count);
  idles_field_.set_text_color(state->non_energized_count ? LV_COLOR_RED
                                                         : LV_COLOR_SILVER);
  if (state->is_energized) {
    const double full_steps = acquisition::state_steps(*state);
    steps_field_.set_text_float(full_steps, 2);
  } else {
    steps_field_.set_text_int(state->full_steps);
  }
}
// Provides common buffer and functionality to capture screens.

#pragma once

#include "analyzer/acquisition.h"
#include "misc/elapsed.h"
#include "ui.h"

namespace capture_util {

  
// Common capture screen controls.
struct CaptureControls {
  // A button to toggle run/stop.
  ui::Button run_button;
  // Run/stop status text.
  ui::Label status_label;

  // Call once on initialization.
  void setup(ui::Screen& screen);

  // Update the button and text label based on common
  // capture state.
 void update_display_from_state();

 void sync_button_to_state();

};

  // Toggle between normal (20ms) and alternative (100ms) scale.
  extern void toggle_scale();

  extern bool alternative_scale() ;

  extern void clear_data();

  extern bool has_data();

  // Returns true if a new data was captured.
  extern bool maybe_update_capture_data();

  // If has_data() is true, this contains the data.
  extern const acquisition::CaptureBuffer* capture_buffer();

  // Given capture controls, update capture enabled/disabled 
  // if needed.
  bool maybe_update_state_from_controls(const CaptureControls& capture_controls);

  extern bool capture_enabled() ;

  extern void set_capture_enabled(bool val) ;

}  // namespace capture_util

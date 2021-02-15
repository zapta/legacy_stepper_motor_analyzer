
#pragma once

#define VERSION_STRING "0.9.12"

namespace config {

// FOR DEBUGGING ONLY. Turn off for official releases.

// For UI debugging. Shows the boundaries of UI objects.
static constexpr bool kDebugBackgrounds = false;

// For developers only. When enabled, clicking on a
// screen's title field pauses the program and sends a screen
// dump over the USB/serial connection.
static constexpr bool kEnableScreenshots = false;

// For developers only. When enabled, clicking on a
// screen's title field generates a Debug event for that
// screen.
static constexpr bool kEnableDebugEvents = false;

}  // namespace config

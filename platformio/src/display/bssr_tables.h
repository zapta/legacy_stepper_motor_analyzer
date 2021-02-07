// Lookup tables to drive the 16 bits parallel output.
#pragma once

#include <arduino.h>

namespace bssr_tables {

extern const uint64_t direct_bssr_table[];
extern const uint64_t color_bssr_table[];

}  // namespace bssr_tables

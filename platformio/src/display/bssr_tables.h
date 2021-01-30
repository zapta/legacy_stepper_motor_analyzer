// Lookup tables to drive the 16 bits parallel output.
#pragma once

#include <arduino.h>


namespace bssr_tables {

// Straight values.
extern const uint32_t direct_bssr_table_port_a[];
extern const uint32_t direct_bssr_table_port_b[];

// With color8 to color16 mapping.
extern const uint32_t color_bssr_table_port_a[];
extern const uint32_t color_bssr_table_port_b[];

}  // namespace bssr_tables

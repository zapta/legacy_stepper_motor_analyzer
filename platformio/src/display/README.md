This directory contains the port of LVGL to the hardware we use. It includes
an ILI9488 480x320 TFT driver, a capacitive touch screen driver and
the LVGL interfaces to these drivers.

To achieve faster refresh rate we use 16 bits data path to the TFT. These
16 output pins are allocated in PORTA and PORTB and we use lookup table
for a fast update of those pins to output a specific 16 bit value.

To reduce memory consumption we also run LVGL in 8 bit color mode and 
use the lookup tables mentioned above to also do the conversion from
LVGL 8 bit colors to the TFT's 16 bit colors.


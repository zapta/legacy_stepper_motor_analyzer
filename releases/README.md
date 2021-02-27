## Simple Stepper Analyzer Releases
This directory contains the official releases of the Simple Stepper Analyzer which are provided as .bin files that can be flashed to the analyzer.

The binaries can be programmed via the USB connector using the builtin STM32 DFU bootloader. The invoke the DFU mode, connect the analyzer to a computer via a USB cable while holding the BOOT0 button at the back of the analyzer.

Unless specified otherwise, the binaries where compiled for a STM32F401CE MCU
(which typically can also run on a STM32F411CE MCU).

&nbsp;

Binary file | version | Comments
:------------ | :------------- | :---------
[firmware-2021-02-26.bin](./firmware-2021-02-26.bin) |  0.9.13 |  Increased buttons' click. area.
[firmware-2021-02-15.bin](./firmware-2021-02-15.bin) |  0.9.12 |  Added phase analysis.
[firmware-2021-02-01.bin](./firmware-2021-02-01.bin) |  0.9.11 |  First released version.


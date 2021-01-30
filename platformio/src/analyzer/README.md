The acquisition direction contains the part of the firmware that 
process the ADC samples and extracts the measurements. All that processing
is done by an interrupt routine that process every 1ms 100 pairs of 
ADC samples that are collected in memory by a DMA channel. The pairs of ADC
samples are triggered by a 100khz timer that is set in bye ../hal/tim.cc file.

Before you change any part of the interrupt routine make sure you know
what you are doing.

The file quadrant_plot.png shows how a single stepper motor cycles
is divided into 8 cases and 4 quadrant representing 4 full steps.
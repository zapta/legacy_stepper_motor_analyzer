// Implementation of the acquisition module. It uses the ADC/DMA
// interrupts to process the ADC sampling.

// TODO: convert const naming style to kCammelCase.

#include "acquisition.h"

#include <stdio.h>
#include <string.h>

#include "filters.h"
#include "hal/adc.h"
#include "hal/dma.h"
#include "hal/gpio.h"

namespace acquisition {

// Updated by sample_state(). This is the frozen copy of the
// state the UI can use.
static State sampled_state;

// 12 bit -> 4096 counts. 3.3V full scale.
// 0.4V per AMP (for +/- 2.5A sensor).
constexpr float kCountsPerAmp = 0.4 * 4096 / 3.3;

// We use this value to do multiplications instead of divisions.
constexpr float kAmpsPerCount = 1 / kCountsPerAmp;

constexpr float kMilliampsPerCount = 1000 / kCountsPerAmp;

// Energized/non-energized histeresis limits in ADC
// counts.
//
// TODO: specify here what the limits are as a percentage
// of full current scale.
//
constexpr uint16_t kNonEnergizedThresholdCounts = 50;
constexpr uint16_t kEnergizedThresholdCounts = 150;

// Hysteresis for determining quadrant transitions. In
// milliamps and in ADC counts.
// constexpr int kQuadrantHisteresisMilliamps = 100;
// constexpr int kQuadrantHysteresisCounts =
//     (kQuadrantHisteresisMilliamps * kCountsPerAmp) / 1000;

// Precompute signed histeresize for transition from the
// current quadrant q = [0, 2, 3, 3].
// static constexpr int quadrant_v1_histeresis[] = {
//     (kQuadrantHysteresisCounts / 2), (-kQuadrantHysteresisCounts / 2),
//     (kQuadrantHysteresisCounts / 2), (-kQuadrantHysteresisCounts / 2)};

// Allowed range for adc zero current offset setting.
// This range is wider than needed and actual offsets
// are expected to be around 1900.
constexpr int kMinOffset = 0;
constexpr int kMaxOffset = 4095;  // 12 bits max

enum CaptureState {
  // Filling half of the capture buffer.
  CAPTURE_HALF_FILL,
  // Keep filling in a circular way until a trigger event
  // or wait for trigger timeout.
  CAPTURE_PRE_TRIGGER,
  // Keep capture points as long as the capture buffer is not full.
  CAPTURE_POST_TRIGER,
  // Not capturing. ISR is guaranteed not to update or access the
  // capture buffer.
  CAPTURE_IDLE,
};

// This data is accessed from interrupt and thus should
// be access from main() with IRQ disabled.
struct IsrData {
  // The state visible to users.
  State state;

  // Current settings.
  Settings settings;

  // Signal capturing.
  // Time out for waiting for trigger in divided ADC ticks.
  uint32_t capture_pre_trigger_items_left = 0;
  // Factor to divide ADC ticks. Only every n'th sample is captured.
  uint16_t capture_divider = 1;
  // Up counter for capturing only every n'th samples.
  uint16_t capture_divider_counter = 0;
  // Capturing state.
  CaptureState capture_state = CAPTURE_IDLE;
  // The capture buffer itself. Updated by ISR when state != CAPTURE_IDLE
  // and accessible by the UI (ready only) when state = CAPTURE_IDLE.
  CaptureBuffer capture_buffer;
};

static IsrData isr_data;

extern bool is_capture_ready() {
  CaptureState capture_state;
  __disable_irq();
  { capture_state = isr_data.capture_state; }
  __enable_irq();
  return capture_state == CAPTURE_IDLE;
}

extern void start_capture(uint16_t divider) {
  // Force a reasonable range.
  if (divider < 1) {
    divider = 1;
  } else if (divider > 1000) {
    divider = 1000;
  }

  // Since capture may be active, data can co-access by ISR.
  __disable_irq();
  {
    isr_data.capture_buffer.items.clear();
    isr_data.capture_buffer.trigger_found = false;
    // This is an arbitrary number of divided samples that we allow
    // to wait for a trigger event.
    isr_data.capture_pre_trigger_items_left = 500;
    isr_data.capture_divider = divider;
    isr_data.capture_divider_counter = 0;
    isr_data.capture_state = CAPTURE_HALF_FILL;
  }
  __enable_irq();
}

// Users are expected to read this buffer only when capturing
// is not active.
const CaptureBuffer* capture_buffer() { return &isr_data.capture_buffer; }

const State* sample_state() {
  __disable_irq();
  sampled_state = isr_data.state;
  __enable_irq();
  return &sampled_state;
}

void reset_state() {
  __disable_irq();
  {
    isr_data.state.tick_count = 0;
    isr_data.state.non_energized_count = 0;
    isr_data.state.full_steps = 0;
    isr_data.state.max_full_steps = 0;
    isr_data.state.max_retraction_steps = 0;
    isr_data.state.quadrature_errors = 0;
    memset(isr_data.state.buckets, 0, sizeof(isr_data.state.buckets));
  }
  __enable_irq();
}

int adc_value_to_milliamps(int adc_value) {
  return (int)(adc_value * kMilliampsPerCount);
}

float adc_value_to_amps(int adc_value) {
  return ((float)adc_value) * kAmpsPerCount;
}

void calibrate_zeros() {
  __disable_irq();
  {
    isr_data.settings.offset1 += isr_data.state.v1;
    isr_data.settings.offset2 += isr_data.state.v2;
  }
  __enable_irq();
}

void set_direction(bool reverse_direction) {
  __disable_irq();
  { isr_data.settings.reverse_direction = reverse_direction; }
  __enable_irq();
}

void get_settings(Settings* settings) {
  __disable_irq();
  { *settings = isr_data.settings; }
  __enable_irq();
}

void dump_sampled_state() {
  // Remember last tick count and report only diff.
  static uint32_t last_tick_count = 0;

  Serial.printf(
      "[%lu][er:%lu] [%5d, %5d] [en:%d %lu] s:%d/%d  steps:%d max_steps:%d\n",
      sampled_state.tick_count - last_tick_count,
      sampled_state.quadrature_errors, sampled_state.v1, sampled_state.v2,
      sampled_state.is_energized, sampled_state.non_energized_count,
      sampled_state.quadrant, sampled_state.last_step_direction,
      sampled_state.full_steps, sampled_state.max_full_steps);

  last_tick_count = sampled_state.tick_count;

  for (int i = 0; i < acquisition::kNumHistogramBuckets; i++) {
    Serial.print(sampled_state.buckets[i].total_ticks_in_steps);
    Serial.print(" ");
  }
  Serial.println();

  Serial.println("...");

  for (int i = 0; i < acquisition::kNumHistogramBuckets; i++) {
    const uint32_t avg_peak_current =
        sampled_state.buckets[i].total_steps
            ? (sampled_state.buckets[i].total_step_peak_currents /
               sampled_state.buckets[i].total_steps)
            : 0;
    Serial.print(avg_peak_current);

    Serial.print(" ");
  }
  Serial.println();
}

// Assumes that capture data is ready.
void dump_capture(const CaptureBuffer& buffer) {
  for (int i = 0; i < buffer.items.size(); i++) {
    const acquisition::CaptureItem* item = buffer.items.get(i);
    // TODO: convert to Serial.printf().
    Serial.print(-15);
    Serial.print(' ');
    Serial.print(item->v1);
    Serial.print(' ');
    Serial.print(item->v2);
    Serial.print(' ');
    Serial.println(15);
  }
}

// Maybe add step's information to the histogram.
// Called from isr on step transition.
inline void isr_add_step_to_histogram(int quadrant, Direction entry_direction,
                                      Direction exit_direction,
                                      uint32_t ticks_in_step,
                                      uint32_t max_current_in_step) {
  // Ignoring this step if not entering and exiting this step in same forward or
  // backward direction.
  if (entry_direction != exit_direction ||
      entry_direction == UNKNOWN_DIRECTION) {
    return;
  }
  uint32_t steps_per_sec =
      TicksPerSecond / ticks_in_step;  // speed in steps per second
  if (steps_per_sec < 10) {
    return;  // ignore very slow steps as they dominate the time.
  }
  uint32_t bucket_index = steps_per_sec / kBucketStepsPerSecond;
  if (bucket_index >= kNumHistogramBuckets) {
    bucket_index = kNumHistogramBuckets - 1;
  }
  HistogramBucket& bucket = isr_data.state.buckets[bucket_index];
  bucket.total_ticks_in_steps += ticks_in_step;
  bucket.total_step_peak_currents += max_current_in_step;
  bucket.total_steps++;
}

// A helper for the isr function.
static inline void isr_update_full_steps_counter(int increment) {
  State& isr_state = isr_data.state;  // alias

  // Update step counter based on direction setting.
  if (isr_data.settings.reverse_direction) {
    isr_state.full_steps -= increment;
  } else {
    isr_state.full_steps += increment;
  }

  // Track retraction.
  if (isr_state.full_steps > isr_state.max_full_steps) {
    isr_state.max_full_steps = isr_state.full_steps;
  }
  const int retraction_steps = isr_state.max_full_steps - isr_state.full_steps;
  if (retraction_steps > isr_state.max_retraction_steps) {
    isr_state.max_retraction_steps = retraction_steps;
  }
}

// NOTE: these four filters slow the interrupt handling. Consider
// to eliminate if free CPU time is insufficient.
//
// We use these filters to reduce internal and external noise.
static filters::Adc12BitsLowPassFilter<700> signal1_filter;
static filters::Adc12BitsLowPassFilter<400> signal2_filter;

// Slow filter, for display purposes.
// static filters::Adc12BitsLowPassFilter<1023> display1_filter;
// static filters::Adc12BitsLowPassFilter<1023> display2_filter;

// This function performs the bulk of the IRQ processing. It accepts
// one pair of ADC1, ADC2 readings, analyzes it, and updates the
// state.
void isr_handle_one_sample(const uint16_t raw_v1, const uint16_t raw_v2) {
  isr_data.state.tick_count++;

  // Fast filtering for signal analysis.
  const int16_t v1 =
      (uint16_t)signal1_filter.update(raw_v1) - isr_data.settings.offset1;
  const int16_t v2 =
      (uint16_t)signal2_filter.update(raw_v2) - isr_data.settings.offset2;

  // Slower filtering for display purposes.
  isr_data.state.v1 = v1;
  //   (uint16_t)display1_filter.update(raw_v1) - isr_data.settings.offset1;
  isr_data.state.v2 = v2;
  // (uint16_t)display2_filter.update(raw_v2) - isr_data.settings.offset2;

  // Handle signal capturing.
  // Release: 220ns, Debug: 1100ns.  (TODO: update timing for current code)
  if (isr_data.capture_state != CAPTURE_IDLE &&
      ++isr_data.capture_divider_counter >= isr_data.capture_divider) {
    isr_data.capture_divider_counter = 0;
    // Insert sample to circular buffer. If the buffer is full it drops
    // the oldest item.
    CaptureItem* capture_item = isr_data.capture_buffer.items.insert();
    capture_item->v1 = v1;
    capture_item->v2 = v2;

    switch (isr_data.capture_state) {
      // In this sate we blindly fill half of the buffer.
      case CAPTURE_HALF_FILL:
        if (isr_data.capture_buffer.items.size() >= kCaptureBufferSize / 2) {
          isr_data.capture_state = CAPTURE_PRE_TRIGGER;
        }
        break;

      // In this state we look for a trigger event or a pre trigger timeout.
      case CAPTURE_PRE_TRIGGER: {
        // Pre trigger timeout?
        if (isr_data.capture_pre_trigger_items_left == 0) {
          // NOTE: if the buffer is full here we could terminate
          // the capture but we go through the normal motions for simplicity.
          isr_data.capture_state = CAPTURE_POST_TRIGER;
          isr_data.capture_buffer.trigger_found = false;

          break;
        }
        isr_data.capture_pre_trigger_items_left--;
        // Trigger event?
        const int16_t old_v1 =
            isr_data.capture_buffer.items.get_reversed(5)->v1;
        // Trigger criteria, up crossing of the zero line.
        if (old_v1 < -10 && v1 >= 0) {
          // Keep only the last n/2 points. This way the trigger will
          // always be in the middle of the buffer.
          isr_data.capture_buffer.items.keep_at_most(kCaptureBufferSize / 2);
          isr_data.capture_buffer.trigger_found = true;
          isr_data.capture_state = CAPTURE_POST_TRIGER;
        }
      } break;

      // In this state we blindly fill the rest of the buffer.
      case CAPTURE_POST_TRIGER:
        if (isr_data.capture_buffer.items.is_full()) {
          isr_data.capture_state = CAPTURE_IDLE;
        }
        break;

      // This one is non reachable but makes the compiler happy.
      case CAPTURE_IDLE:
        break;
    }
  }

  // Determine if motor is energized. Use hysteresis for noise rejection.
  // Release: 200ns. Debug: 600ns.
  const bool old_is_energized = isr_data.state.is_energized;
  const uint16_t total_current = abs(v1) + abs(v2);
  // Using histeresis.
  const uint16_t energized_threshold = old_is_energized
                                           ? kNonEnergizedThresholdCounts
                                           : kEnergizedThresholdCounts;
  const bool new_is_energized = total_current > energized_threshold;
  isr_data.state.is_energized = new_is_energized;

  // Handle the non energized case. No need to go through quadrant decoding.
  // Pass through case: Release: 110ns. Debug: 250ns.
  if (!new_is_energized) {
    if (old_is_energized) {
      // Becoming non energized.
      isr_data.state.last_step_direction = UNKNOWN_DIRECTION;
      isr_data.state.ticks_in_step = 0;
      isr_data.state.non_energized_count++;
    } else {
      // Staying non energized
    }
    return;
  }

  // Here when energized. Decode quadrant.
  // We now go through a decision tree to collect the new quadrant, sector
  // and max coil current. Optimized for speed. See quadrants_plot.png
  // for the individual cases.
  uint8_t new_quadrant;  // set below to [0, 3]
  uint32_t max_current;  // max coil current
  if (v2 >= 0) {
    if (v1 >= 0) {
      // Quadrant 0: v1 > 0, V2 > 0.
      new_quadrant = 0;
      if (v1 > v2) {
        // Sector 0: v1 > 0, V2 > 0.  |v1| > |v2|
        max_current = v1;
      } else {
        // Sector 1: v1 > 0, V2 > 0.  |v1| < |v2|
        max_current = v2;
      }
    } else {
      // Quadrant 1: v1 < 0, V2 > 0
      new_quadrant = 1;
      if (-v1 < v2) {
        // Sector 2: v1 < 0, V2 > 0.  |v1| < |v2|
        max_current = v2;
      } else {
        // Sector 3: v1 < 0, V2 > 0.  |v1| > |v2|
        max_current = -v1;
      }
    }
  } else {
    if (v1 < 0) {
      // Quadrant 2:  v1 < 0, V2 < 0
      new_quadrant = 2;
      if (-v1 > -v2) {
        // Sector 4: v1 < 0, V2 < 0.  |v1| > |v2|
        max_current = -v1;
      } else {
        // Sector 5: v1 < 0, V2 < 0.  |v1| < |v2|
        max_current = -v2;
      }
    } else {
      // Quadrant 3 v1 > 0, V2 < 0.
      new_quadrant = 3;
      if (v1 < -v2) {
        // Sector 6: v1 > 1, V2 < 0.  |v1| < |v2|
        max_current = -v2;
      } else {
        // Sector 7: v1 > 0, V2 < 0.  |v1| > |v2|
        max_current = v1;
      }
    }
  }

  const int8_t old_quadrant = isr_data.state.quadrant;  // old quadrant [0, 3]
  isr_data.state.quadrant = new_quadrant;

  // Track quadrant transitions and update steps.
  if (!old_is_energized) {
    // Case 1: motor just became energized. Direction is still not known.
    isr_data.state.last_step_direction = UNKNOWN_DIRECTION;
    isr_data.state.ticks_in_step = 1;
    isr_data.state.max_current_in_step = max_current;
  } else if (new_quadrant == old_quadrant) {
    // Case 2: staying in same quadrant
    isr_data.state.ticks_in_step++;
    if (max_current > isr_data.state.max_current_in_step) {
      isr_data.state.max_current_in_step = max_current;
    }
  } else if (new_quadrant == ((old_quadrant + 1) & 0x03)) {
    // Case 3: Moved to next quadrant.
    isr_update_full_steps_counter(+1);
    isr_add_step_to_histogram(old_quadrant, isr_data.state.last_step_direction,
                              FORWARD, isr_data.state.ticks_in_step,
                              isr_data.state.max_current_in_step);
    isr_data.state.last_step_direction = FORWARD;
    isr_data.state.ticks_in_step = 1;
    isr_data.state.max_current_in_step = max_current;
  } else if (new_quadrant == ((old_quadrant - 1) & 0x03)) {
    // Case 4: Moved to previous quadrant.
    isr_update_full_steps_counter(-1);
    isr_add_step_to_histogram(old_quadrant, isr_data.state.last_step_direction,
                              BACKWARD, isr_data.state.ticks_in_step,
                              isr_data.state.max_current_in_step);
    isr_data.state.last_step_direction = BACKWARD;
    isr_data.state.ticks_in_step = 1;
    isr_data.state.max_current_in_step = max_current;
  } else {
    // Case 5: Invalid quadrant transition.
    // TODO: count and report errors.
    isr_data.state.quadrature_errors++;
    isr_data.state.last_step_direction = UNKNOWN_DIRECTION;
    isr_data.state.ticks_in_step = 1;
    isr_data.state.max_current_in_step = max_current;
  }
}

// Handle the first or second ADC/DMA buffer with collected
// samples.
void isr_handle_dma_buffer(const dma::AdcPoint* bfr, int n) {
  for (int i = 0; i < n; i++) {
    LED2_ON;
    const dma::AdcPoint& adc_point = bfr[i];
    isr_handle_one_sample(adc_point.v1, adc_point.v2);
    LED2_OFF;
  }
}

// HAL interrupt handler for the ADC/DMA 'half' completion.
// We process the data in buffer 1.
extern "C" void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc) {
  isr_handle_dma_buffer(dma::kDmaAdcPointBuffer1, dma::kDmaAdcPointBufferSize);
}

// HAL interrupt handler for the ADC/DMA 'full' completion.
// We process the data in buffer 2.
extern "C" void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
  isr_handle_dma_buffer(dma::kDmaAdcPointBuffer2, dma::kDmaAdcPointBufferSize);
}

// Force a valid settings offset value.
static int clip_offset(int requested_offset) {
  return max(kMinOffset, min(kMaxOffset, requested_offset));
}

// Call once on program initialization.
void setup(const Settings& settings) {
  isr_data.settings = settings;
  isr_data.settings.offset1 = clip_offset(isr_data.settings.offset1);
  isr_data.settings.offset2 = clip_offset(isr_data.settings.offset2);
}

// This involves floating point operations and thus slow. Do not
// call from the interrupt routine.
double state_steps(const State& state) {
  // If not energized, we can't compute fractional steps.
  if (!state.is_energized) {
    return state.full_steps;
  }

  // Compute fractional stept. We use the abs() to avoid
  // the discontinuity near -180 degrees. It provides better safety with
  // the non determinism of floating point values.
  // Range is in [0, PI];
  const double abs_radians = abs(atan2(state.v2, state.v1));

  // Rel radians in [-PI/4, PI/4].
  double rel_radians = 0;
  switch (state.quadrant) {
    case 0:
      rel_radians = abs_radians - (PI / 4);
      break;
    case 1:
      rel_radians = abs_radians - (3 * PI / 4);
      break;
    case 2:
      rel_radians = (3 * PI / 4) - abs_radians;
      break;
    case 3:
      rel_radians = (PI / 4) - abs_radians;
      break;
  }

  // Fraction is in the range [-0.5, 0.5]
  const double fraction = rel_radians * (2 / PI);

  // NOTE: this is a little bit hacky since we don't know the direction
  // flag setting at the time this sample was captured but should
  // be good enough for now.
  //
  // TODO: record last direction flag value in the state.
  const double result = isr_data.settings.reverse_direction
                            ? state.full_steps - fraction
                            : state.full_steps + fraction;

  // Serial.printf("%d, %hu, (%hd, %hd), %d, ", state.full_steps,
  // state.quadrant,
  //               state.v1, state.v2, isr_data.settings.reverse_direction);
  // Serial.print(abs_radians);
  // Serial.print(", ");
  // Serial.print(rel_radians);
  // Serial.print(", ");
  // Serial.println(fraction);

  return result;
}

}  // namespace acquisition

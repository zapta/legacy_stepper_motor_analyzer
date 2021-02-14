#include "capture_util.h"

namespace capture_util {

SharedCaptureState SharedCaptureState::instance;

static constexpr uint32_t kUpdateIntervalMillis = 500;

static constexpr uint16_t kCaptureDividerNormal = 10;

static constexpr uint16_t kCaptureDividerAlternative = 50;

bool SharedCaptureState::maybe_update_capture_data() {
    // Is disabled?
  if (!capture_enabled_) {
    return false;
  }

  // Do we need to start a new capture?
  if (!capture_in_progress_) {
    if (elapsed_from_last_update_.elapsed_millis() >= kUpdateIntervalMillis) {
      // This will prevent the timer from overflowing, without affecting
      // the logic here.
      elapsed_from_last_update_.set(kUpdateIntervalMillis);
      acquisition::start_capture(alternative_scale_ ? kCaptureDividerAlternative
                                                   : kCaptureDividerNormal);
      capture_in_progress_ = true;
    }
    return false;
  }

  // Capture in progress but data is not ready yet.
  if (!acquisition::is_capture_ready()) {
    return false;
  }

  // Here where capture is enablled and the last
  // capture request was just completed.
  capture_in_progress_ = false;

  // Since capture is not in progress now, the content of the
  // asquisition capture buffer should be stable and it's safe to use
  // it.
  const acquisition::CaptureBuffer* acq_capture_buffer =
      acquisition::capture_buffer();
  // const acquisition::CaptureItems* items = &capture_buffer->items;

  // Currently we display only captured with a trigger point.
  // This seens to give a better user experience.
  if (!acq_capture_buffer->trigger_found) {
    return false;
  }

  // Here we are comitted to update with the new capture.
  capture_buffer_ = *acq_capture_buffer;
  has_data_ = true;
  elapsed_from_last_update_.reset();
  return true;
  // SharedCaptureState::instance.data_alternative_scale =
  //    SharedCaptureState::instance.capture_alternative_scale;
}

}  // namespace capture_util

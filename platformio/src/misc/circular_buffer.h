

#pragma once

#include <Arduino.h>

// Implements a cyclic array with a static max size. Used in 
// IRQ routines and thus optimized for speed.
template <class T, uint16_t n>
class CircularBuffer {
    public:
  CircularBuffer()  { clear(); }

  inline uint16_t capacity() const { return n; };

  inline bool is_full() const  { return size_ >= n; }

  inline bool is_empty() const { return size_ == 0; }

  inline void clear() {
    next_ = 0;
    size_ = 0;
  }

  inline uint16_t size() const {  return size_; }

  // Retuns a pointer to the new location to initialized.
  inline T* insert() {
      T* result = &items_[next_];
    if (++next_ >= n) {
      next_ = 0;
    } 
    // Increase size if was not full. Otherwise oldest item is 
    // dropped.
    if (size_ < n) {
      size_++;
    }
    return result;
  }

  // Index i should be < size(). 
  inline const T* get(uint16_t i) const {
    uint16_t i1 = next_ + i;
    uint16_t i2 = (i1 >= size_) ? i1 - size_ : i1 - size_ + n;
    return &items_[i2];
  }

  // Index i should be < size().
  inline const T* get_reversed(uint16_t i) const {
    uint16_t i1 = (next_ > i) ? next_ - 1 - i : next_ + n - 1 - i;
    return &items_[i1];
  }


  // Keep up to this number of newest items.
  inline void keep_at_most(uint16_t max_size) {
      // Nothing to do.
      if (size_ <= max_size) {
          return;
      }
      size_ = max_size;
  }

 private:
  // Next insertion index. In [0, n).
  uint16_t next_ = 0;
  // Actual number of items. In [0, n].
  uint16_t size_;
  // Items buffer.
  T items_[n];
};
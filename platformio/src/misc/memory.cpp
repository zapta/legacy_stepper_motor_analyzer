
#include "memory.h"

namespace memory {
int free_memory() {
#ifndef __arm__
#error "Unexpected architecture"
#endif  // __arm__
  char stack_bottom;
  const char* heap_top = reinterpret_cast<char*>(sbrk(0));
  return &stack_bottom - heap_top;
}

}  // namespace memory

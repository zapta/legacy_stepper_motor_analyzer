// For debugging. Reports unused memory between above heap and 
// below stack.
//
// See Arduino core at
// ~/AppData/Local/Arduino15/packages/arduino/hardware/megaavr/1.8.6/cores/arduino

#pragma once

#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else   // __ARM__
extern char *__brkval;
#endif  // __arm__

namespace memory {
extern int free_memory();

} // namespace memory
 


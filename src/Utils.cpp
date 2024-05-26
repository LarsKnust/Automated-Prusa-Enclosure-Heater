#include <Arduino.h>
# ifdef WATCHDOG
#include <avr/wdt.h>
# endif
#include <configuration.h>

#include <Utils.hpp>

#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__

int freeMemory() {
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  return &top - __brkval;
#else  // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}

void enableWatchdog()
{
# ifdef WATCHDOG
  // Enable the watchdog with a timeout of 4 seconds
  wdt_enable(WDTO_4S);
# endif
}

void resetWatchdog()
{
# ifdef WATCHDOG
  // Reset the watchdog 
  wdt_reset();
# endif
}

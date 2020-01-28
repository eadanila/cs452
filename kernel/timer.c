#include "timer.h"

void read_debug_timer(int *high, int *low) {
    *low = *DEBUG_TIMER_LOW;
    *high = *DEBUG_TIMER_HIGH & DEBUG_TIMER_HIGH_MASK;
}

void stop_debug_timer(void) {
    *DEBUG_TIMER_HIGH = *DEBUG_TIMER_HIGH & !DEBUG_TIMER_CTRL;
}

void start_debug_timer(void) {
    *DEBUG_TIMER_HIGH = *DEBUG_TIMER_HIGH | DEBUG_TIMER_CTRL;
}

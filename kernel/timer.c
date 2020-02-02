#include "timer.h"

unsigned int read_debug_timer(void) {
    return *DEBUG_TIMER_LOW;
}

void stop_debug_timer(void) {
    *DEBUG_TIMER_HIGH = 0x0;
}

void start_debug_timer(void) {
    *DEBUG_TIMER_HIGH = 0x100;
}

void stop_tc1(void) {
    *(TC1_CTRL) &= ~(TC1_ENABLE_MASK);
}

void start_tc1(void) {
    *(TC1_LOAD) = 71;
    *(TC1_CTRL) |= (TC1_ENABLE_MASK);
}


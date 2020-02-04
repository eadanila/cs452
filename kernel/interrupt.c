#include "interrupt.h"
#include "timer.h"
#include "logging.h"

void handle_interrupt(void) {
    // get reg boi 0x800B0000 and 0x800C0000
    // check which bit is enabled for interrupt handling
    
    uint irq_status_lo = *((unsigned int *) 0x800B0000);

    switch (irq_status_lo) {
    case 0x10: // 2^4
        clear_timer(TIMER_TC1);
        return;
        break;
    case 0x20: // 2^5
        clear_timer(TIMER_TC2);
        return;
        break;
    default:
        break;
    }

    uint irq_status_hi = *((unsigned int *) 0x800C0000);
    switch(irq_status_hi) {
    case 0x80000:
        clear_timer(TIMER_TC3);
        return;
        break;
    default:
        break;
    }
    
    FATAL("Unknown interrupt - lo: %x, hi: %x", irq_status_lo, irq_status_hi);
}

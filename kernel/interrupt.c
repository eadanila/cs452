#include "interrupt.h"
#include "timer.h"
#include "logging.h"

void enable_interrupt(uint interrupt) {
    assert(interrupt <= 63);

    switch (interrupt) {
    case INTERRUPT_TC1UI:
        *INTERRUPT_VIC1_ENABLE_ADDR |= 1 << INTERRUPT_TC1UI;
        break;
    case INTERRUPT_TC2UI:
        *INTERRUPT_VIC1_ENABLE_ADDR |= 1 << INTERRUPT_TC2UI;
        break;
    case INTERRUPT_TC3UI:
        *INTERRUPT_VIC2_ENABLE_ADDR |= 1 << (INTERRUPT_TC3UI - 32);
        break;
    default:
        FATAL("Attempt to enable unhandled interrupt: %d", interrupt);
        break;
    }
}


void disable_interrupt(uint interrupt) {
    assert(interrupt <= 63);

    if (interrupt < 32) {
        *INTERRUPT_VIC1_CLEAR_ADDR |= 1 << interrupt;
    } else {
        *INTERRUPT_VIC2_CLEAR_ADDR |= 1 << (interrupt - 32);
    }
}


void clear_vic(void) {
    *INTERRUPT_VIC1_CLEAR_ADDR = 0xFFFFFFFF;
    *INTERRUPT_VIC2_CLEAR_ADDR = 0xFFFFFFFF;
}


void handle_interrupt(void) {
    LOG("handle_interrupt called");
    
    uint irq_status_lo = *INTERRUPT_VIC1_STATUS_ADDR;

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

    uint irq_status_hi = *INTERRUPT_VIC2_STATUS_ADDR;
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
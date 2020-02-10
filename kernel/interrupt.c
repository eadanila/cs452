#include "interrupt.h"
#include "timer.h"
#include "logging.h"

#include "await.h"
#include "pqueue.h"
#include "task.h"
#include "frame.h"

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


void handle_interrupt(uint runner) {
    LOG("handle_interrupt called");

    int tid;
    Frame *fp;

    set_task_state(runner, TASK_READY);
    push_task(runner);
    
    uint irq_status_lo = *INTERRUPT_VIC1_STATUS_ADDR;

    switch (irq_status_lo) {
    case 0x10: // 2^4
        tid = event_wake(EVENT_TIMER1_INTERRUPT);

        if (tid > -1)  {
            fp = (Frame *)get_task_stack_pointer(tid);
            fp->r0 = read_timer(TIMER_TC3);
            set_task_state(tid, TASK_READY);
            push_task(tid);
        }

        clear_timer(TIMER_TC1);

        return;
        break;
    case 0x20: // 2^5
        tid = event_wake(EVENT_TIMER2_INTERRUPT);

        if (tid > -1)  {
            fp = (Frame *)get_task_stack_pointer(tid);
            fp->r0 = read_timer(TIMER_TC2);
            set_task_state(tid, TASK_READY);
            push_task(tid);
        }

        clear_timer(TIMER_TC2);

        return;
        break;
    default:
        break;
    }

    uint irq_status_hi = *INTERRUPT_VIC2_STATUS_ADDR;
    switch(irq_status_hi) {
    case 0x80000:
        tid = event_wake(EVENT_TIMER3_INTERRUPT);

        if (tid > -1)  {
            fp = (Frame *)get_task_stack_pointer(tid);
            fp->r0 = read_timer(TIMER_TC3);
            set_task_state(tid, TASK_READY);
            push_task(tid);
        }

        clear_timer(TIMER_TC3);

        return;
        break;
    default:
        break;
    }
    
    FATAL("Unknown interrupt - lo: %x, hi: %x", irq_status_lo, irq_status_hi);
}

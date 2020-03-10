#include "interrupt.h"
#include "timer.h"
#include "logging.h"

#include "await.h"
#include "pqueue.h"
#include "task.h"
#include "frame.h"

#include "uart.h"
#include "idle_printer.h"

void enable_interrupt(uint interrupt) {
    assert(interrupt <= 63);

    switch (interrupt) {
    case INTERRUPT_TC1UI:
    case INTERRUPT_TC2UI:
    case INTERRUPT_UART1RXINTR1:
    case INTERRUPT_UART1TXINTR1:
    case INTERRUPT_UART2TXINTR2:
    case INTERRUPT_UART2RXINTR2:
        *INTERRUPT_VIC1_ENABLE_ADDR |= 1 << interrupt;
        break;

    case INTERRUPT_TC3UI:
    case INTERRUPT_UART1:
    case INTERRUPT_UART2:
        *INTERRUPT_VIC2_ENABLE_ADDR |= 1 << (interrupt - 32);
        break;

    default:
        WARN("Attempt to enable unknown interrupt: %d", interrupt);
        if (interrupt < 32)
            *INTERRUPT_VIC1_ENABLE_ADDR |= 1 << interrupt;
        else
            *INTERRUPT_VIC2_ENABLE_ADDR |= 1 << (interrupt - 32);
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


uint handle_vic1(void) {
    int tid;
    Frame *fp;

    uint vic = *INTERRUPT_VIC1_STATUS_ADDR;
    uint handled = 0;
    uint interrupt = 0;

    // TC1 underflow interrupt
    interrupt = (1 << INTERRUPT_TC1UI);
    if (vic & interrupt) {
        tid = event_wake(EVENT_TIMER1_INTERRUPT);
        if (tid > -1)  {
            fp = (Frame *)get_task_stack_pointer(tid);
            fp->r0 = read_timer(TIMER_TC3);
            set_task_state(tid, TASK_READY);
            push_task(tid);
        }
        clear_timer(TIMER_TC1);
        handled |= interrupt;

        add_measurement(&idle_buffer, idle_time);
        idle_time = 0;
    }

    // TC2 underflow interrupt
    interrupt = (1 << INTERRUPT_TC2UI);
    if (vic & interrupt) {
        tid = event_wake(EVENT_TIMER2_INTERRUPT);
        if (tid > -1)  {
            fp = (Frame *)get_task_stack_pointer(tid);
            fp->r0 = read_timer(TIMER_TC2);
            set_task_state(tid, TASK_READY);
            push_task(tid);
        }
        clear_timer(TIMER_TC2);
        handled |= interrupt;
    }

    // UART1 RX interrupt
    interrupt = (1 << INTERRUPT_UART1RXINTR1);
    if (vic & interrupt) {
        tid = event_wake(EVENT_UART1_RX_INTERRUPT);
        if (tid > -1) {
            fp = (Frame *)get_task_stack_pointer(tid);
            fp->r0 = uart_read_byte(UART1);
            disable_uart_rx_interrupt(UART1);
            set_task_state(tid, TASK_READY);
            push_task(tid);
        } else {
            FATAL("Interrupt UART1RX that nobody wants");
        }
        handled |= interrupt;
    }

    // UART1 TX interrupt
    interrupt = (1 << INTERRUPT_UART1TXINTR1);
    if (vic & interrupt) {
        tid = event_wake(EVENT_UART1_TX_INTERRUPT);
        if (tid > -1) {
            fp = (Frame *)get_task_stack_pointer(tid);
            fp->r0 = read_uart_flag(UART1, UART_TXFE_FLAG);
            disable_uart_tx_interrupt(UART1);
            set_task_state(tid, TASK_READY);
            push_task(tid);
        } else {
            FATAL("Interrupt UART1TX that nobody wants");
        }
        // empty handler
        handled |= interrupt;
    }

    // UART2 RX interrupt
    interrupt = (1 << INTERRUPT_UART2RXINTR2);
    if (vic & interrupt) {
        tid = event_wake(EVENT_UART2_RX_INTERRUPT);
        if (tid > -1) {
            fp = (Frame *)get_task_stack_pointer(tid);
            fp->r0 = uart_read_byte(UART2);
            disable_uart_rx_interrupt(UART2);
            set_task_state(tid, TASK_READY);
            push_task(tid);
        } else {
            FATAL("Interrupt UART2RX that nobody wants");
        }
        handled |= interrupt;
    }

    // UART2 TX interrupt
    interrupt = (1 << INTERRUPT_UART2TXINTR2);
    if (vic & interrupt) {
        tid = event_wake(EVENT_UART2_TX_INTERRUPT);
        if (tid > -1) {
            fp = (Frame *)get_task_stack_pointer(tid);
            fp->r0 = uart_read_byte(UART2);
            disable_uart_tx_interrupt(UART2);
            set_task_state(tid, TASK_READY);
            push_task(tid);
        } else {
            FATAL("Interrupt UART2TX that nobody wants");
        }
        handled |= interrupt;
    }

    DEBUG("Got VIC1 interrupts: 0x%x. Handled: 0x%x.", vic, handled);

    return (vic - handled);
}

void handle_uart_combined_interrupt(int id) {
    assert(id == UART1 || id == UART2);

    int tid;
    Frame *fp;
    uint flag;

    uint event_high, event_low;
    
    WARN("combined interrupt");

    switch(id) {
    case(UART1):
        event_high = EVENT_UART1_CTS_HIGH;
        event_low = EVENT_UART1_CTS_LOW;
        break;
    default:
        FATAL("UART %d has no combined interrups", id);
        return;
    }

    flag = read_uart_flag(id, UART_CTS_FLAG);
    if (flag) {
        tid = event_wake(event_high);
        if (tid > -1) {
            fp = (Frame *)get_task_stack_pointer(tid);
            fp->r0 = flag;
            set_task_state(tid, TASK_READY);
            push_task(tid);
        }
    } else {
        tid = event_wake(event_low);
        if (tid > -1) {
            fp = (Frame *)get_task_stack_pointer(tid);
            fp->r0 = flag;
            set_task_state(tid, TASK_READY);
            push_task(tid);
        }
    }

    clear_uart_combined_interrupt(id);
}

uint handle_vic2(void) {
    int tid;
    Frame *fp;

    uint vic = *INTERRUPT_VIC2_STATUS_ADDR;
    uint handled = 0;
    uint interrupt = 0;

    // TC3 underflow interrupt
    interrupt = (1 << (INTERRUPT_TC3UI-32));
    if (vic & interrupt) {
        tid = event_wake(EVENT_TIMER3_INTERRUPT);
        if (tid > -1)  {
            fp = (Frame *)get_task_stack_pointer(tid);
            fp->r0 = read_timer(TIMER_TC3);
            set_task_state(tid, TASK_READY);
            push_task(tid);
        }
        clear_timer(TIMER_TC3);
        handled |= interrupt;
    }

    // UART1 combined interrupt
    interrupt = (1 << (INTERRUPT_UART1-32));
    if (vic & interrupt) {
        handle_uart_combined_interrupt(UART1);
        disable_uart_combined_interrupt(UART1);
        handled |= interrupt;
    }

    // UART2 combined interrupt
    interrupt = (1 << (INTERRUPT_UART2-32));
    if (vic & interrupt) {
        // handle_uart_combined_interrupt(UART2);
        // disable_uart_combined_interrupt(UART2);
        handled |= interrupt;
    }

    DEBUG("Got VIC2 interrupts: 0x%x. Handled: 0x%x.", vic, handled);

    return (vic - handled);
}


void handle_interrupt(uint runner) {
    LOG("handle_interrupt called");

    set_task_state(runner, TASK_READY);
    push_task(runner);
    
    uint vic1 = handle_vic1();
    uint vic2 = handle_vic2();

    if (vic1 > 0 || vic2 > 0)
        FATAL("Unknown interrupts - lo: 0x%x, hi: 0x%x", vic1, vic2);
}


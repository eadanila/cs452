#include "await.h"
#include "constants.h"
#include "task.h"
#include "logging.h"
#include "pqueue.h"
#include "uart.h"

int event_wait_tid[EVENT_MAX];
volatile uint *event_interrupt_mask[EVENT_MAX];


void init_event_wait_tid_list(void) {
    for (int i = 0; i < EVENT_MAX; i++) {
        event_wait_tid[i] = TASK_INVALID;
    }
}


int event_wake(int eventid) {
    assert(eventid < EVENT_MAX);
    uint tid = event_wait_tid[eventid];
    event_wait_tid[eventid] = TASK_INVALID;
    return tid;
}


int event_await(int eventid, uint tid) {
    assert(eventid < EVENT_MAX);
    assert(tid < MAX_TASKS_ALLOWED);
    assert(event_wait_tid[eventid] == TASK_INVALID);
    
    if (eventid >= EVENT_MAX)
        return -1;

    // check states and enable interrupts as needed
    switch(eventid) {
    case EVENT_UART1_RX_INTERRUPT:
        enable_uart_rx_interrupt(UART1);
        break;
    case EVENT_UART1_TX_INTERRUPT:
        enable_uart_tx_interrupt(UART1);
        break;
    case EVENT_UART1_CTS_LOW:
        if (!(read_uart_flag(UART1, UART_CTS_FLAG)))
            return 1;
        enable_uart_combined_interrupt(UART1);
        break;
    case EVENT_UART1_CTS_HIGH:
        if (read_uart_flag(UART1, UART_CTS_FLAG))
            return 1;
        enable_uart_combined_interrupt(UART1);
        break;
    case EVENT_UART2_RX_INTERRUPT:
        enable_uart_rx_interrupt(UART2);
        break;
    case EVENT_UART2_TX_INTERRUPT:
        enable_uart_tx_interrupt(UART2);
        break;
    }

    event_wait_tid[eventid] = tid;

    return 0;
}


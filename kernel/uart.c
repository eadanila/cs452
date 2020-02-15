#include "uart.h"
#include "constants.h"
#include "logging.h"

void enable_uart_tx_interrupt(int id) {
    assert(id == UART1 || id == UART2);

    switch(id) {
    case UART1:
        *UART1_CTRL_ADDR |= UART_TIE_MASK;
        break;
    case UART2:
        *UART2_CTRL_ADDR |= UART_TIE_MASK;
        break;
    default:
        FATAL("Unknown UART number: %d", id);
    }
}

void disable_uart_tx_interrupt(int id) {
    assert(id == UART1 || id == UART2);

    switch(id) {
    case UART1:
        *UART1_CTRL_ADDR &= ~UART_TIE_MASK;
        break;
    case UART2:
        *UART2_CTRL_ADDR &= ~UART_TIE_MASK;
        break;
    default:
        FATAL("Unknown UART number: %d", id);
    }
}

void enable_uart_rx_interrupt(int id) {
    assert(id == UART1 || id == UART2);

    switch(id) {
    case UART1:
        *UART1_CTRL_ADDR |= UART_RIE_MASK;
        break;
    case UART2:
        *UART2_CTRL_ADDR |= UART_RIE_MASK;
        break;
    default:
        FATAL("Unknown UART number: %d", id);
    }
}

void disable_uart_rx_interrupt(int id) {
    assert(id == UART1 || id == UART2);

    switch(id) {
    case UART1:
        *UART1_CTRL_ADDR &= ~UART_RIE_MASK;
        break;
    case UART2:
        *UART2_CTRL_ADDR &= ~UART_RIE_MASK;
        break;
    default:
        FATAL("Unknown UART number: %d", id);
    }
}

char uart_read_byte(int id) {
    assert(id == UART1 || id == UART2);

    switch(id) {
    case UART1:
        return (char) *UART1_DATA_ADDR;
    case UART2:
        return (char) *UART2_DATA_ADDR;
    default:
        FATAL("Unknown UART number: %d", id);
    }

    return 0;
}

void uart_send_byte(int id, char byte) {
    assert(id == UART1 || id == UART2);

    switch(id) {
    case UART1:
        *UART1_DATA_ADDR = (uint)byte;
        break;
    case UART2:
        *UART2_DATA_ADDR = (uint)byte;
        break;
    default:
        FATAL("Unknown UART number: %d", id);
    }
}


void enable_uart_combined_interrupt(int id) {
    assert(id == UART1 || id == UART2);

    switch(id) {
    case UART1:
        *UART1_CTRL_ADDR |= UART_MSIE_MASK;
        break;
    case UART2:
        *UART2_CTRL_ADDR |= UART_MSIE_MASK;
        break;
    default:
        FATAL("Unknown UART number: %d", id);
    }
}

void disable_uart_combined_interrupt(int id) {
    assert(id == UART1 || id == UART2);

    switch(id) {
    case UART1:
        *UART1_CTRL_ADDR &= ~UART_MSIE_MASK;
        break;
    case UART2:
        *UART2_CTRL_ADDR &= ~UART_MSIE_MASK;
        break;
    default:
        FATAL("Unknown UART number: %d", id);
        return;
    }
}

uint uart_read_combined_interrupt(int id) {
    assert(id == UART1 || id == UART2);

    switch(id) {
    case UART1:
        return *UART1_INT_ID_INT_CLR_ADDR;
    case UART2:
        return *UART2_INT_ID_INT_CLR_ADDR;
    default:
        FATAL("Unknown UART number: %d", id);
        return 0xFFFFFFFF;
    }
}

void clear_uart_combined_interrupt(int id) {
    assert(id == UART1 || id == UART2);

    switch(id) {
    case UART1:
        *UART1_INT_ID_INT_CLR_ADDR = 0;
        break;
    case UART2:
        *UART2_INT_ID_INT_CLR_ADDR = 0;
        break;
    default:
        FATAL("Unknown UART number: %d", id);
    }
}

int read_uart_flag(int id, int flag) {
    assert(id == UART1 || id == UART2);
    assert(flag == 1 || ((flag % 2 == 0) && flag <= 128));
    
    uint flags = uart_status(id);
    if (flags == 0xFFFFFFFF)
        return -1;
    else
        return (int)(uart_status(id) & flag);
}

uint uart_status(int id) {
    assert(id == UART1 || id == UART2);

    switch(id) {
    case UART1:
        return *UART1_FLAG_ADDR;
    case UART2:
        return *UART2_FLAG_ADDR;
    default:
        FATAL("Unknown UART number: %d", id);
        return 0xFFFFFFFF;
    }
}


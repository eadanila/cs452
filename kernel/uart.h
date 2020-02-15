#ifndef UART_H
#define UART_H

#include "constants.h"

#define UART1 1
    #define UART1_DATA_ADDR             ((volatile uint *)0x808C0000)
    #define UART1_RX_STS_ADDR           ((volatile uint *)0x808C0004)
    #define UART1_LIN_CTRL_HIGH_ADDR    ((volatile uint *)0x808C0008)
    #define UART1_LIN_CTRL_MID_ADDR     ((volatile uint *)0x808C000C)
    #define UART1_LIN_CTRL_LOW_ADDR     ((volatile uint *)0x808C0010)
    #define UART1_CTRL_ADDR             ((volatile uint *)0x808C0014)
    #define UART1_FLAG_ADDR             ((volatile uint *)0x808C0018)
    #define UART1_INT_ID_INT_CLR_ADDR   ((volatile uint *)0x808C001C)
#define UART2 2
    #define UART2_DATA_ADDR             ((volatile uint *)0x808D0000)
    #define UART2_RX_STS_ADDR           ((volatile uint *)0x808D0004)
    #define UART2_LIN_CTRL_HIGH_ADDR    ((volatile uint *)0x808D0008)
    #define UART2_LIN_CTRL_MID_ADDR     ((volatile uint *)0x808D000C)
    #define UART2_LIN_CTRL_LOW_ADDR     ((volatile uint *)0x808D0010)
    #define UART2_CTRL_ADDR             ((volatile uint *)0x808D0014)
    #define UART2_FLAG_ADDR             ((volatile uint *)0x808D0018)
    #define UART2_INT_ID_INT_CLR_ADDR   ((volatile uint *)0x808D001C)


// FLAG masks
#define UART_TXFE_FLAG 128
#define UART_RXFF_FLAG 64
#define UART_TXFF_FLAG 32
#define UART_RXFE_FLAG 16
#define UART_BUSY_FLAG 8
#define UART_DCD_FLAG  4
#define UART_DSR_FLAG  2
#define UART_CTS_FLAG  1

// Interrupt Enable masks
#define UART_TIE_MASK  32
#define UART_RIE_MASK  16
#define UART_MSIE_MASK 8

// Interrupt Status masks
#define UART_TIS_MASK  4
#define UART_RIS_MASK  2
#define UART_MIS_MASK  1

// Require: id is a UART number
//  Result: sets the TIE bit in UART_CTRL register
void enable_uart_tx_interrupt(int id);

// Require: id is a UART number
//  Result: unsets the TIE bit in UART_CTRL register
void disable_uart_tx_interrupt(int id);

// Require: id is a UART number
//  Result: sets the RIE bit in UART_CTRL register
void enable_uart_rx_interrupt(int id);

// Require: id is a UART number
//  Result: unsets the RIE bit in UART_CTRL register
void disable_uart_rx_interrupt(int id);

// Require: id is a UART number
//  Return: byte on specified UART
char uart_read_byte(int id);

void uart_send_byte(int id, char byte);

// Require: id is a UART number
//  Result: sets the MSIE bit in UART_CTRL register
void enable_uart_combined_interrupt(int id);

// Require: id is a UART number
//  Result: unsets the MSIE bit in UART_CTRL register
void disable_uart_combined_interrupt(int id);

// Require: id is a UART number
//  Result: returns the contents of the INT_ID_CLR register
uint read_uart_combined_interrupt(int id);

// Require: id is a UART number
//  Result: writes to the INT_ID_CLR register
void clear_uart_combined_interrupt(int id);

// Require: id is a UART number, flag is a FLAG field
//  Return: state of the specified flag
int read_uart_flag(int id, int flag);

// Require: id is a UART number
//  Return: full state of the UART_FLAG register
uint uart_status(int id);

#endif


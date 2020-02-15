#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "constants.h"

#define INTERRUPT_VIC1_STATUS_ADDR ((volatile unsigned int *) 0x800B0000)
#define INTERRUPT_VIC1_ENABLE_ADDR ((volatile unsigned int *) 0x800B0010)
#define INTERRUPT_VIC1_CLEAR_ADDR  ((volatile unsigned int *) 0x800B0014)

#define INTERRUPT_VIC2_STATUS_ADDR ((volatile unsigned int *) 0x800C0000)
#define INTERRUPT_VIC2_ENABLE_ADDR ((volatile unsigned int *) 0x800C0010)
#define INTERRUPT_VIC2_CLEAR_ADDR  ((volatile unsigned int *) 0x800C0014)

#define INTERRUPT_TC1UI 4
#define INTERRUPT_TC2UI 5
#define INTERRUPT_TC3UI 51

#define INTERRUPT_UART1RXINTR1 23
#define INTERRUPT_UART1TXINTR1 24
#define INTERRUPT_UART1 52

#define INTERRUPT_UART2RXINTR2 25
#define INTERRUPT_UART2TXINTR2 26
#define INTERRUPT_UART2 54


// Require: 0 <= interrupt < 64
//  Result: sets the corresponding bit in the VIC.
// Note: explicitly checks that the specified interrupt is supported
// by the kernel and maps it to the corresponding VIC1 or VIC2 bit
void enable_interrupt(uint interrupt);

// Require: 0 <= interrupt < 64
//  Result: sets the corresponding bit in the VIC.
// Note: does not check if the interrupt is supported by the kernel.
// If interrupt < 32, sets the corresponding bit on VIC1, otherwise
// sets the corresponding bit on VIC2.
void disable_interrupt(uint interrupt);

// Both VIC1 and VIC2 are cleared completely: i.e. VIC1_CLEAR and
// VIC2_CLEAR are set to 0xFFFFFFFF.
void clear_vic(void);

// Called by the kernel to handle interrupts. Does all the probing of
// interrupt addresses itself.
void handle_interrupt(uint runner);

#endif


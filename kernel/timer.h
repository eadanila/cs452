#ifndef TIMER_H
#define TIMER_H

#include "constants.h"

#define TIMER_TC1 1
#define TIMER_TC2 2
#define TIMER_TC3 3
#define TIMER_TC4 4

#define TIMER_1_BASE 0x80810000
    #define TIMER_1_LOAD_REGISTER    ((volatile unsigned int *) (TIMER_1_BASE))
    #define TIMER_1_VALUE_REGISTER   ((volatile unsigned int *) (TIMER_1_BASE + 0x4))
    #define TIMER_1_CONTROL_REGISTER ((volatile unsigned int *) (TIMER_1_BASE + 0x8))
    #define TIMER_1_CLEAR_REGISTER   ((volatile unsigned int *) (TIMER_1_BASE + 0xC))

#define TIMER_2_BASE 0x80810020
    #define TIMER_2_LOAD_REGISTER    ((volatile unsigned int *) (TIMER_2_BASE))
    #define TIMER_2_VALUE_REGISTER   ((volatile unsigned int *) (TIMER_2_BASE + 0x4))
    #define TIMER_2_CONTROL_REGISTER ((volatile unsigned int *) (TIMER_2_BASE + 0x8))
    #define TIMER_2_CLEAR_REGISTER   ((volatile unsigned int *) (TIMER_2_BASE + 0xC))

#define TIMER_3_BASE  0x80810080
    #define TIMER_3_LOAD_REGISTER    ((volatile unsigned int *) (TIMER_3_BASE))
    #define TIMER_3_VALUE_REGISTER   ((volatile unsigned int *) (TIMER_3_BASE + 0x4))
    #define TIMER_3_CONTROL_REGISTER ((volatile unsigned int *) (TIMER_3_BASE + 0x8))
    #define TIMER_3_CLEAR_REGISTER   ((volatile unsigned int *) (TIMER_3_BASE + 0xC))

#define TIMER_ENABLE (unsigned int) 0x80 // 1 << 7
#define TIMER_MODE   (unsigned int) 0x40 // 1 << 6
#define TIMER_CLKSEL (unsigned int) 0x08 // 1 << 3

#define DEBUG_TIMER_HIGH (volatile unsigned int *) 0x80810064
#define DEBUG_TIMER_LOW  (volatile unsigned int *) 0x80810060
#define DEBUG_TIMER_CTRL 0x100 // 1 << 8
#define DEBUG_TIMER_HIGH_MASK 0xFF // first 8 bits

// Return: value read form DEBUG_TIMER_LOW address.
unsigned int read_debug_timer(void);

// Result: Writes 0x0 (disable all bits) to DEBUG_TIMER_HIGH
void stop_debug_timer(void);

// Result: Writes 0x100 (enable bit set) to DEBUG_TIMER_HIGH
void start_debug_timer(void);

// Require: timer_id is one of TIMER_TC1, TIMER_TC2, TIMER_TC3 or TIMER_TC4
//  Result: unsets the TIMER_ENABLE bit in the timer's CONTROL_REGISTER for
//  TIMER_TC1, TIMER_TC2, and TIMER_TC3. Calls stop_debug_timer() for
//  TIMER_TC4.
void disable_timer(uint timer_id);

// Require: timer_id is one of TIMER_TC1, TIMER_TC2, TIMER_TC3 or TIMER_TC4
//  Result: sets the TIMER_ENABLE bit in the timer's CONTROL_REGISTER for
//  TIMER_TC1, TIMER_TC2, and TIMER_TC3. Calls start_debug_timer() for
//  TIMER_TC4.
void enable_timer(uint timer_id);

// Require: timer_id is one of TIMER_TC1, TIMER_TC2, TIMER_TC3
//  Result: writes to the timer's CLEAR_REGISTER to clear any generated
//  interrupt
void clear_timer(uint timer_id);

// Require: timer_id is one of TIMER_TC1, TIMER_TC2, TIMER_TC3 and
// mode is 0 or 1
//  Result: sets TIMER_MODE on the timer if mode is 1, unsets it
//  if mode is 0
void set_timer_mode(uint timer_id, uint mode);

// Require: timer_id is one of TIMER_TC1, TIMER_TC2, TIMER_TC3 and
// clock is 0 or 1
//  Result: sets TIMER_CLKSEL on the timer if clock is 1, unsets it
//  if clock is 0
void set_timer_clock(uint timer_id, uint clock);

// Require: timer_id is one of TIMER_TC1, TIMER_TC2, TIMER_TC3 and
// value <= 0xFFFF if timer_id is TIMER_TC1 or TIMER_TC2
void set_timer_load_value(uint timer_id, uint value);

// Require: 0 < timer_id <= 4
//  Return: number of ticks in the timer value register
uint read_timer(uint timer_id);

#endif


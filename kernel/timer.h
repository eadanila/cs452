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

unsigned int read_debug_timer(void);
void stop_debug_timer(void);
void start_debug_timer(void);

void disable_timer(int timer_id);
void enable_timer(int timer_id);
void clear_timer(int timer_id);
void set_timer_mode(int timer_id, int mode);
void set_timer_clock(int timer_id, int clock);
void set_timer_load_value(int timer_id, int value);
uint read_timer(int timer_id);

#endif


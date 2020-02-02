#ifndef TIMER_H
#define TIMER_H

#define DEBUG_TIMER_HIGH (volatile unsigned int *) 0x80810064
#define DEBUG_TIMER_LOW  (volatile unsigned int *) 0x80810060
#define DEBUG_TIMER_CTRL 0x100 // 1 << 8
#define DEBUG_TIMER_HIGH_MASK 0xFF // first 8 bits

unsigned int read_debug_timer(void);
void stop_debug_timer(void);
void start_debug_timer(void);

#define TC1_CTRL (volatile unsigned int *) 0x80810008
#define TC1_ENABLE_MASK (unsigned int) 0x80
#define TC1_LOAD (volatile unsigned int *) 0x80810000
void stop_tc1(void);
void start_tc1(void);

#endif


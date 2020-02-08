#ifndef AWAIT_H
#define AWAIT_H

#include "constants.h"

#define EVENT_MAX 3 // number of the largest event, update as more are added
#define EVENT_TIMER1_INTERRUPT 1
#define EVENT_TIMER2_INTERRUPT 2
#define EVENT_TIMER3_INTERRUPT 3

void init_event_wait_tid_list(void);
int event_wake(int eventid);
int event_await(int eventid, uint tid);

#endif


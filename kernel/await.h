#ifndef AWAIT_H
#define AWAIT_H

// await supports the client-sever model of event waiting. It has a
// wait list with a spot for each waitable event. This means that only
// one task can wait on an event at once, so notifying multiple tasks
// that an event has occurred should happen via a server and message
// passing.

#include "constants.h"

#define EVENT_TIMER1_INTERRUPT 0
#define EVENT_TIMER2_INTERRUPT 1
#define EVENT_TIMER3_INTERRUPT 2

#define EVENT_UART1_RX_INTERRUPT 3
#define EVENT_UART1_TX_INTERRUPT 4
#define EVENT_UART1_CTS_LOW     5
#define EVENT_UART1_CTS_HIGH    6

#define EVENT_UART2_TX_INTERRUPT 7
#define EVENT_UART2_RX_INTERRUPT 8

#define EVENT_MAX 9 // number of the largest event, update as more are added

// Require: this is called before anything else in await
//  Result: the array of TIDs waiting on events is intialized
void init_event_wait_tid_list(void);

// Require: eventid is a defined event
//  Return: the task ID of the task waiting on that event, -1 if no 
//  task is waiting.
// The task will be removed from the wait list. This should be called
// by handle_interrupt to wake tasks that called AwaitEvent(eventid)
int event_wake(int eventid);

// Require: eventid is a defined event, tid is a valid task, no task is
// already waiting on that event.
//  Return: -1 if event is invalid. handle_interrupt is responsible for
//  setting return codes for tasks that successfuly await
// The task is set to TASK_AWAIT and the tid is saved in the wait list
int event_await(int eventid, uint tid);

#endif


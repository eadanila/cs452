#include "await.h"
#include "constants.h"
#include "task.h"
#include "logging.h"
#include "pqueue.h"

int event_wait_tid[EVENT_MAX];


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

    event_wait_tid[eventid] = tid;

    return 0;
}


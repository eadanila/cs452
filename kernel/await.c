#include "await.h"
#include "constants.h"
#include "task.h"
#include "logging.h"
#include "pqueue.h"

int event_wait_tid[EVENT_MAX+1];


void init_event_wait_tid_list(void) {
    for (int i = 0; i <= EVENT_MAX; i++) {
        event_wait_tid[i] = -1;
    }
}


int event_wake(int eventid) {
    uint tid = event_wait_tid[eventid];
    event_wait_tid[eventid] = -1;
    return tid;
}


int event_await(int eventid, uint tid) {
    assert(eventid <= EVENT_MAX);
    assert(tid < MAX_TASKS_ALLOWED);
    
    if (eventid > EVENT_MAX)
        return -1;

    event_wait_tid[eventid] = tid;

    return 0;
}


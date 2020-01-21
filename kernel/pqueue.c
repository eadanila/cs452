#include "pqueue.h"
#include "kernel.h"

void init_pqueue() {
    // decided to set head = tail = -1 for empty queue
    // can be changed to something else, only here for sanity
    // we should always be checking size for emptyiness anyway
    for (int i = 0; i < MIN_PRIORITY; i++) {
        task_schedule[i].head = -1;
        task_schedule[i].tail = -1;
        task_schedule[i].size = 0;
    }
}

void add_task(int id, int pri) {
    if (task_schedule[pri].size == 0) {
        // in a 1-item queue, head = tail
        task_schedule[pri].head = id;
        task_schedule[pri].tail = id;
    } else {
        // set the current tail's next to the new item ID
        // then set the tail to the new item
        int tail = task_schedule[pri].tail;
        tasks[tail].next = id;
        tasks[id].next = -1;
        task_schedule[pri].tail = id;
    }
    task_schedule[pri].size += 1;
} 

int pop_task(int pri) {
    // noting to pop
    if (task_schedule[pri].size == 0)
        return -1;

    // head at least is something since the queue is non-empty
    int head = task_schedule[pri].head;
    int next = -1;

    // if the queue had 2+ items, next will be the head's next ID
    // if the queue has only 1 item, tail must also become -1
    if (task_schedule[pri].size >= 2) {
        next = tasks[head].next;
    } else {
        task_schedule[pri].tail = -1;
    }

    // head = either -1 or the next of the previous head
    task_schedule[pri].head = next;
    task_schedule[pri].size -= 1;

    if (task_schedule[pri].size == 0) {
        task_schedule[pri].head = -1;
        task_schedule[pri].head = -1;
    }

    return head;
}

int front_task(int pri) {
    // -1 if empty queue
    // head otherwise
    if (task_schedule[pri].size == 0)
        return -1;
    return task_schedule[pri].head;
}

int get_current_priority(void) {
    for (int i = 0; i < MIN_PRIORITY; i++) {
        if (task_schedule[i].head > 0)
            return i;
    }
    return -1;
}

// TODO Change -1 in returns to 0 and uint 
// Define task id of 0 as invalid?
int next_scheduled_task()
{
    return pop_task(get_current_priority());
}

void cycle_schedule(int pri)
{
    if (task_schedule[pri].size > 1)
        add_task(pop_task(pri), pri);
}

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

void remove_task(int id, int pri) {
    // assert(id == task_schedule[pri].head)
    // assert(task_schedule[pri].size > 0)

    // if the task is at the front, logic is same as pop and discard
    if (task_schedule[pri].head == id) {
        pop_task(pri);
        return ;  
    }

    // the queue only has one element, but it is not ID
    if (task_schedule[pri].size == 1)
        return ; // should we return something to indicate ID not in queue
    
    // if we got here, the removed ID is at least the second element
    // cur is initialized to that element, then we search for ID in the queue
    int cur = tasks[task_schedule[pri].head].next;
    int prev = task_schedule[pri].head;
    while (cur != id) {
        prev = cur;
        cur = tasks[id].next;
        if (!tasks[cur].is_valid)
            return ;
    }
    // we've found the tasks we're looking for, stitch the queue over it
    tasks[prev].next = tasks[cur].next;
    task_schedule[pri].size -= 1;
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


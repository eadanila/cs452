#ifndef PQUEUE_H
#define PQUEUE_H

#include "constants.h"

struct pqueue
{
    int head; // task id of head task
    int tail; // task id of tail task
    int size; // number of tasks in queue
};
typedef struct pqueue PQUEUE;

void init_pqueue(void);

// add_task and pop_task are FIFO
void add_task(int id, int pri);
void remove_task(int id, int pri);
int pop_task(int pri);
int front_task(int pri);

int get_current_priority(void);
// Obtains the next task on the highest priority queue
int next_scheduled_task(); 
// Takes the next task on the highest prioirty queue, and places
// it at the back of its queue
void cycle_schedule(); 

#endif


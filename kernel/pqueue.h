#ifndef PQUEUE_H
#define PQUEUE_H


#include "constants.h"

struct pqueue
{
    int queues[MAX_TASKS_ALLOWED][MIN_PRIORITY+1];
    uint queue_starts[MIN_PRIORITY+1];
    uint queue_ends[MIN_PRIORITY+1];
    uint queue_sizes[MIN_PRIORITY+1];
};

typedef struct pqueue pqueue;

void init_pqueue(struct pqueue* pq);

// add_task and remove_task are FIFO
void add_task(pqueue* pq, int id, int priority);
int remove_task(pqueue* pq, int priority);
int front_task(pqueue* pq, int priority);
uint pq_size(pqueue* pq, int priority);

#endif
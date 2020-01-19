#include "pqueue.h"

void init_pqueue(struct pqueue* pq)
{
    // Currently the queues must be initialized within kmain 
    for(int i = 0; i != MIN_PRIORITY+1; i++) pq->queue_starts[i] = 0;
    for(int i = 0; i != MIN_PRIORITY+1; i++) pq->queue_ends[i] = 0;
    for(int i = 0; i != MIN_PRIORITY+1; i++) pq->queue_sizes[i] = 0;
}

void add_task(pqueue* pq, int id, int priority)
{
    if(id >= MAX_TASKS_ALLOWED) return;
    
    pq->queue_ends[priority] = (pq->queue_ends[priority] + 1)%(MIN_PRIORITY+1);
    pq->queues[priority][pq->queue_ends[priority]] = id;
    pq->queue_sizes[priority]++;
}

int remove_task(pqueue* pq, int priority)
{
    if(pq->queue_sizes[priority] == 0) return 0; // TODO Define 0 as invalid task

    int r = pq->queues[priority][pq->queue_starts[priority]];
    pq->queue_starts[priority] = (pq->queue_starts[priority] + 1)%(MIN_PRIORITY+1);
    pq->queue_sizes[priority]--;
    
    return r;
}

int front_task(pqueue* pq, int priority)
{   
    if(pq->queue_sizes[priority] == 0) return 0;
    return pq->queues[priority][pq->queue_starts[priority]];
}

uint pq_size(pqueue* pq, int priority)
{
    return pq->queue_sizes[priority];
}
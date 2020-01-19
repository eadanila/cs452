#ifndef PQUEUE_H
#define PQUEUE_H

#include "constants.h"

typedef struct pqueue pqueue;
typedef struct pq_node pq_node;

struct pq_node
{
    uint id;
    uint priority;
    pq_node* prev;
    pq_node* next;
};

struct pqueue
{
    pq_node nodes[MAX_TASKS_ALLOWED*(MIN_PRIORITY+1)];
    pq_node *id_to_node[MAX_TASKS_ALLOWED];
    pq_node *available_nodes[MAX_TASKS_ALLOWED*(MIN_PRIORITY+1)];
    int next_available_node;

    pq_node *queue_fronts[MIN_PRIORITY+1];
    pq_node *queue_ends[MIN_PRIORITY+1];
    uint queue_sizes[MIN_PRIORITY+1];
};

void init_pqueue(struct pqueue* pq);

// add_task and pop_task are FIFO
void add_task(pqueue* pq, int id, int priority);
void remove_task(pqueue* pq, int id, int priority);
int pop_task(pqueue* pq, int priority);
int front_task(pqueue* pq, int priority);
uint pq_size(pqueue* pq, int priority);

// OLD PQUEUE IMPLEMENTATION
// struct pqueue
// {
//     int queues[MAX_TASKS_ALLOWED][MIN_PRIORITY+1];
//     uint queue_starts[MIN_PRIORITY+1];
//     uint queue_ends[MIN_PRIORITY+1];
//     uint queue_sizes[MIN_PRIORITY+1];
// };

#endif

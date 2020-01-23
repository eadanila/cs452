#ifndef PQUEUE_H
#define PQUEUE_H

#include "constants.h"

typedef struct pqueue PQUEUE;
typedef struct pqueue_node PQUEUE_NODE;
typedef struct pqueue_queue PQUEUE_QUEUE;

struct pqueue_node
{
    uint id;
    PQUEUE_NODE *next;
};

struct pqueue_queue
{
    PQUEUE_NODE* head;
    PQUEUE_NODE* tail;
};

struct pqueue
{
    // Array holding all nodes to be used for all priority queues
    struct pqueue_node data[MAX_TASKS_ALLOWED];
    // Next free pqueue_node, if its 0 then all nodes 
    // have been allocated and we are out of tasks
    PQUEUE_NODE *free;
    // Each prioirty level queue
    PQUEUE_QUEUE queues[PRIORITY_COUNT];
};


// Initializes the global pqueue, 
// must be called before calling any of the following functions!
void init_pqueue();

// Pushes task onto the queue corresponding to its priority.
// Require: id is a valid task
void push_task(uint id);

// Removes the first task on the highest priorty level queue
// and returns it.
// Return: a valid task id or -1 if the queue was empty.
int pop_task();

// Returns the task on the highest priorty level queue
// Return: a valid task id or -1 if the queue was empty.
int peek_task();

#endif


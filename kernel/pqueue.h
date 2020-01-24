#ifndef PQUEUE_H
#define PQUEUE_H

#include "constants.h"

typedef struct pqueue PQueue;
typedef struct pqueue_node PQueue_Node;
typedef struct pqueue_queue PQueue_Queue;

struct pqueue_node
{
    uint id;
    PQueue_Node *next;
};

struct pqueue_queue
{
    PQueue_Node* head;
    PQueue_Node* tail;
};

struct pqueue
{
    // Array holding all nodes to be used for all priority queues
    PQueue_Node data[MAX_TASKS_ALLOWED];
    // Next free pqueue_node, if its 0 then all nodes 
    // have been allocated and we are out of tasks
    PQueue_Node *free;
    // Each prioirty level queue
    PQueue_Queue queues[PRIORITY_COUNT];
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


#ifndef MESSAGEQUEUE_H
#define MESSAGEQUEUE_H

#include "constants.h"

typedef struct id_queue      IDQueue;
typedef struct id_node       IDNode;
typedef struct message_queue MessageQueue;

struct id_node
{
    uint id;
    IDNode *next;
};

struct id_queue
{
    IDNode *head;
    IDNode *tail;
};

struct message_queue
{
    // Array holding all nodes to be used for all message queues
    IDNode data[MAX_TASKS_ALLOWED];

    // Next free IDNode, if its 0 then all nodes 
    // have been allocated and we are out of tasks.
    IDNode *free;

    // Each task's message queue
    IDQueue queues[MAX_TASKS_ALLOWED];
};


// Initializes the global MessageQueue, 
// must be called before calling any of the following functions!
void init_message_queue();

// Pushes messaging task onto the id's message queue.
// Require: id is a valid task
void push_message(int id, int m_id);

// Removes the first messaging task on the id's queue and returns it.
// Return: a valid task id or -1 if the queue was empty.
int pop_message(int id);

// Returns the first messaging task on the id's queue
// Return: a valid task id or -1 if the queue was empty.
int peek_message(int id);

#endif


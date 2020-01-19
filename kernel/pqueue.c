#include "pqueue.h"

void init_pqueue(struct pqueue* pq)
{
    pq->next_available_node = MAX_TASKS_ALLOWED*(MIN_PRIORITY+1) -1;
    for(int i = 0; i != MAX_TASKS_ALLOWED*(MIN_PRIORITY+1); i++) pq->available_nodes[i] = &(pq->nodes[i]);
    for(int i = 0; i != MAX_TASKS_ALLOWED; i++) pq->id_to_node[i] = 0;
    
    for(int i = 0; i != MIN_PRIORITY+1; i++) pq->queue_fronts[i] = 0;
    for(int i = 0; i != MIN_PRIORITY+1; i++) pq->queue_ends[i] = 0;
    for(int i = 0; i != MIN_PRIORITY+1; i++) pq->queue_sizes[i] = 0;
}

void add_task(pqueue* pq, int id, int priority)
{
    if(id >= MAX_TASKS_ALLOWED) return;
    if(pq->next_available_node == -1) return;
    
    // "Allocate" new pq_node
    pq_node* new_node = pq->available_nodes[pq->next_available_node];
    pq->next_available_node--;
    pq->id_to_node[id] = new_node;

    new_node->id = id;
    new_node->priority = priority;
    new_node->next = 0;
    new_node->prev = pq->queue_ends[priority];
    
    if(pq->queue_fronts[priority] == 0) pq->queue_fronts[priority] = new_node;
    
    pq->queue_ends[priority] = new_node;
    pq->queue_sizes[priority]++;
}

void remove_task(pqueue* pq, int id, int priority)
{
    pq_node* node = pq->id_to_node[id];

    if(node == 0) return;

    if(node->next && node->prev)
    {
        node->next->prev = node->prev;
        node->prev->next = node->next;
    }
    else if(node->next) // node->prev == 0
    {
        pq->queue_fronts[priority] = node->next;
        node->next->prev = 0;
    }
    else if(node->prev) // node->next == 0
    {
        pq->queue_ends[priority] = node->prev;
        node->prev->next = 0;
    }
    else
    {
        pq->queue_ends[priority] = 0;
        pq->queue_fronts[priority] = 0;
    }

    // "Deallocate" the node
    pq->next_available_node++;
    pq->available_nodes[pq->next_available_node] = node;
    pq->id_to_node[node->id] = 0;
    pq->queue_sizes[priority]--;
}

int pop_task(pqueue* pq, int priority)
{
    if(pq->queue_fronts[priority] == 0) return 0;
    
    pq_node* node = pq->queue_fronts[priority];
    int r = node->id;

    if(node->next) node->next->prev = 0;
    pq->queue_fronts[priority] = node->next;

    pq->next_available_node++;
    pq->available_nodes[pq->next_available_node] = node;
    pq->id_to_node[node->id] = 0;
    pq->queue_sizes[priority]--;

    return r;
}

int front_task(pqueue* pq, int priority)
{   
    if(pq->queue_fronts[priority] == 0) return 0;
    return pq->queue_fronts[priority]->id;
}

uint pq_size(pqueue* pq, int priority)
{
    return pq->queue_sizes[priority];
}

// OLD PQUEUE IMPLEMENTATION
// void init_pqueue(struct pqueue* pq)
// {
//     // Currently the queues must be initialized within kmain 
//     for(int i = 0; i != MIN_PRIORITY+1; i++) pq->queue_starts[i] = 0;
//     for(int i = 0; i != MIN_PRIORITY+1; i++) pq->queue_ends[i] = 0;
//     for(int i = 0; i != MIN_PRIORITY+1; i++) pq->queue_sizes[i] = 0;
// }

// void add_task(pqueue* pq, int id, int priority)
// {
//     if(id >= MAX_TASKS_ALLOWED) return;
    
//     pq->queue_ends[priority] = (pq->queue_ends[priority] + 1)%(MIN_PRIORITY+1);
//     pq->queues[priority][pq->queue_ends[priority]] = id;
//     pq->queue_sizes[priority]++;
// }

// int remove_task(pqueue* pq, int priority)
// {
//     if(pq->queue_sizes[priority] == 0) return 0; // TODO Define 0 as invalid task

//     int r = pq->queues[priority][pq->queue_starts[priority]];
//     pq->queue_starts[priority] = (pq->queue_starts[priority] + 1)%(MIN_PRIORITY+1);
//     pq->queue_sizes[priority]--;
    
//     return r;
// }

// int front_task(pqueue* pq, int priority)
// {   
//     if(pq->queue_sizes[priority] == 0) return 0;
//     return pq->queues[priority][pq->queue_starts[priority]];
// }

// uint pq_size(pqueue* pq, int priority)
// {
//     return pq->queue_sizes[priority];
// }

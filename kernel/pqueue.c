#include "pqueue.h"
#include "kernel.h"
#include "task.h"

#include "logging.h"

PQueue task_schedule;

void init_pqueue() {
    // Initialize all nodes as free
    for (int i = 0; i < MAX_TASKS_ALLOWED - 1; i++) task_schedule.data[i].next = &task_schedule.data[i+1];
    task_schedule.data[MAX_TASKS_ALLOWED-1].next = 0;

    for (int i = 0; i < PRIORITY_COUNT; i++)
    {
        task_schedule.queues[i].head = 0;
        task_schedule.queues[i].tail = 0;
    } 
    task_schedule.free = &task_schedule.data[0];
}

// Return the highest priority level occupied by a task.
// Return priority 0 if all priority levels are unoccupied.
int highest_priorty()
{
    for (int i = 0; i < PRIORITY_COUNT; i++) { if(task_schedule.queues[i].head != 0) return i; }
    return 0;
}

void push_task(uint id)
{
    assert(task_schedule.free != 0) // Ran out of tasks!
    assert(is_valid_task(id));

    int pri = get_task_by_id(id).priority;
    DEBUG("Adding task %d to queue %d", id, pri);

    PQueue_Node *tail = task_schedule.queues[pri].tail;
    PQueue_Node *new_node = task_schedule.free;
    task_schedule.free = task_schedule.free->next;
    
    new_node->id = id;
    new_node->next = 0;
    if(tail == 0)
    {
        // Empty queue
        task_schedule.queues[pri].tail = new_node;
        task_schedule.queues[pri].head = new_node;
    } 
    else
    {
        task_schedule.queues[pri].tail->next = new_node;
        task_schedule.queues[pri].tail = new_node;
    } 
}

int pop_task()
{
    int pri = highest_priorty();
    if(task_schedule.queues[pri].head == 0) return -1; // No tasks on any priorty queue
    int id = task_schedule.queues[pri].head->id;

    PQueue_Node *new_head = task_schedule.queues[pri].head->next;
    task_schedule.queues[pri].head->next = task_schedule.free;
    task_schedule.free = task_schedule.queues[pri].head;
    task_schedule.queues[pri].head = new_head;
    if(new_head == 0) task_schedule.queues[pri].tail = 0;

    return id;
}

int peek_task()
{
    int pri = highest_priorty();
    if(task_schedule.queues[pri].head == 0) return -1; // No tasks on any priorty queue
    int id = task_schedule.queues[pri].head->id;

    return id;
}


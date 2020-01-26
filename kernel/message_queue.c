#include "message_queue.h"
#include "kernel.h"

#include "logging.h"

MessageQueue m;

void init_message_queue() {
    // Initialize all nodes as free
    for (int i = 0; i < MAX_TASKS_ALLOWED - 1; i++) m.data[i].next = &m.data[i+1];
    m.data[MAX_TASKS_ALLOWED-1].next = 0;

    for (int i = 0; i < MAX_TASKS_ALLOWED; i++)
    {
        m.queues[i].head = 0;
        m.queues[i].tail = 0;
    } 
    m.free = &m.data[0];
}

void push_message(int id, int m_id)
{
    assert(m.free != 0) // Ran out of tasks!

    DEBUG("Adding messaging task %d to %d's message queue", m_id, id);

    IDNode *tail = m.queues[id].tail;
    IDNode *new_node = m.free;
    m.free = m.free->next;
    
    new_node->id = id;
    new_node->next = 0;
    if(tail == 0)
        // Empty queue
        m.queues[id].head = new_node;
    else
        m.queues[id].tail->next = new_node;
    
    m.queues[id].tail = new_node;
}

int pop_message(int id)
{
    if(m.queues[id].head == 0) return -1; // No tasks on any priorty queue
    int m_id = m.queues[id].head->id;

    IDNode *new_head = m.queues[id].head->next;
    m.queues[id].head->next = m.free;
    m.free = m.queues[id].head;
    m.queues[id].head = new_head;
    if(new_head == 0) m.queues[id].tail = 0;

    return m_id;
}

int peek_message(int id)
{
    if(m.queues[id].head == 0) return -1; // No tasks on any priorty queue
    return m.queues[id].head->id;
}


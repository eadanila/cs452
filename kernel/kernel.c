#include <bwio.h>

#include "kernel.h"
#include "arm_lib.h"
#include "syscall.h"
#include "pqueue.h"

void fuck(void);

void print_lr(uint u) {
    bwprintf(COM2, "shit's broke: %x\r\n", u);
    while (1);
}

int next_task_id(void) {
    // For now, do not try to fill in holes.
    if(next_task + 1 >= MAX_TASKS_ALLOWED) return OUT_OF_TASK_DESCRIPTORS;

    next_task++;
    task_count++;
    return next_task;
}

void schedule(void) {
    int pri = get_current_priority();
    if (pri == -1)
        return;
    add_task(pop_task(pri), pri);

    // manually find and print highest priority queue for debug
    for (int i = 0; i < MIN_PRIORITY; i++) {
        if (task_schedule[i].size > 0) {
            int id = task_schedule[i].head;
            #if DEBUG_ON
            bwprintf(COM2, "Priority = %d\r\n", i);
            #endif
            while(id != -1) {
                #if DEBUG_ON
                bwprintf(COM2, "%d -> ", id);
                #endif
                id = tasks[id].next;
            }
            #if DEBUG_ON
            bwprintf(COM2, "E\r\n");
            #endif
            return ;
        }
    }
}

int Create(int priority, void (*function)())
{
    int id = next_task_id();
    
    if(id == OUT_OF_TASK_DESCRIPTORS) 
    {
        bwprintf(COM2, "\r\nMaximum number of tasks reached!\r\n");
        return id;
    }
    
    uint *stack_base = (uint *)(MEMORY_START+id*TASK_MEMORY_SIZE);

    struct frame *fr = (struct frame *)(stack_base - 16);
    tasks[id].is_valid = 1;
    tasks[id].t_id = id;
    tasks[id].p_id = 0; // TODO Find way to pass down parent id.
    tasks[id].stack_base = stack_base;
    tasks[id].stack_pointer = stack_base - 16;
    tasks[id].pc = function;

    // TODO Remove
    uint *p = ((uint *)stack_base) - 16;
    for (int i = 0; i < 17; i++) {
        *(p+i) = 0;
    }

    fr->r15 = (uint)function;
    fr->r13 = (uint)stack_base;
    fr->r14 = (uint)exit_handler;
    fr->cspr = (uint)CSPR_USER_MODE;

    add_task(id, priority);

    #if DEBUG_ON
    // TODO Remove
    for (int i = 0; i < 17; i++) {
        bwprintf(COM2, "i:%d,&i:%x\r\n", i, *(p + i));
    }

    bwprintf(COM2, "\r\nInitialized new task %d.\r\n", id);
    #endif

    return id;
}

void kinit() {
    // Initialize COM2
    bwsetspeed(COM2, 115200);
    bwsetfifo(COM2, OFF);

    // Initialize kernel constants
    task_count = 0;
    next_task = 0;
    init_pqueue();
    
    for(int id = 0; id < MAX_TASKS_ALLOWED; id++) tasks[id].is_valid = 0;

    uint *p = (uint *)0x20;
    for (int i = 0; i < 8; i++) {
        *p = (uint)fuck;
        p = p + 1;
    }
    uint *handler_dest = (uint *)0x28;
    *handler_dest = (uint)enter_kernel;


    // init kernel task
    tasks[0].t_id = 0;
    tasks[0].p_id = 0;
    tasks[0].stack_pointer = (uint *)0x01000000;
    tasks[0].pc = 0x0; // what should this be for the kernel?
    #if DEBUG_ON
    bwprintf(COM2, "\r\nKERNEL!\r\n");
    #endif
}


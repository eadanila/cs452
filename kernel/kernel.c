#include <bwio.h>

#include "kernel.h"
#include "arm_lib.h"
#include "syscall.h"
#include "pqueue.h"

#include "logging.h"
#include "task.h"

void unhandled_exception_handler(void);

void print_lr(uint u) {
    bwprintf(COM2, "Uh oh, stinkyyy: %x\r\n", u);
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
                id = get_task_next_id(id);
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
    int p_id = get_running_task();
    int id = allocate_task(p_id, priority, function);
    
    if(id == OUT_OF_TASK_DESCRIPTORS) 
    {
        WARN("Maximum number of tasks reached!");
        return id;
    }

    task t = get_task_by_id(id);
    struct frame *f = (struct frame *)(t.stack_base - 16);

    f->r13 = (uint)t.stack_base;
    f->r15 = (uint)function;
    f->r14 = (uint)exit_handler;
    f->cspr = (uint)CSPR_USER_MODE;

    set_task_stack_pointer(id, (uint *)t.stack_base - 16);

    add_task(id, priority);

    print("Created: %d\r\n", id);

    return id;
}

// TODO: inline assembly to asm_lib
int user_mode() {
    asm("mrs r0, cpsr");
    register int *proc asm("r0");
    int mode = (int)proc & 0x1F;
    return mode == 0x10;
}

void panic() {
    // something bad has happened and now the kernel is in a panic
    // print panic message and return to redboot
    bwprintf(COM2, "\r\n");
    bwprintf(COM2, "Did you ever hear the tragedy of Darth Plagueis The Wise?\r\n");
    bwprintf(COM2, "I thought not. It’s not a story the Jedi would tell you.\r\n");
    bwprintf(COM2, "It’s a Sith legend. Darth Plagueis was a Dark Lord of the\r\n");
    bwprintf(COM2, "Sith, so powerful and so wise he could use the Force to\r\n");
    bwprintf(COM2, "influence the midichlorians to create life. He had such a\r\n");
    bwprintf(COM2, "knowledge of the dark side that he could even keep the\r\n");
    bwprintf(COM2, "ones he cared about from dying. The dark side of the\r\n");
    bwprintf(COM2, "Force is a pathway to many abilities some consider to be\r\n");
    bwprintf(COM2, "unnatural. He became so powerful the only thing he was\r\n");
    bwprintf(COM2, "afraid of was losing his power, which eventually, of\r\n");
    bwprintf(COM2, "course, he did. Unfortunately, he taught his apprentice\r\n");
    bwprintf(COM2, "everything he knew, then his apprentice killed him in his\r\n");
    bwprintf(COM2, "sleep.\r\n\r\n");

    bwprintf(COM2, "Ironic. He could save others from death, but not himself.\r\n");

    // TODO: inline assembly to asm_lib
    // load the redboot address into lr and return to it
    // redboot return address is 0x174C8
    // ARMv4 has a limit on how large an immediate value
    // can be moved into a register, so we need to load
    // 0x174, shift 8 bits, then add 0xC8
    asm("mov r0, #0x174");
    asm("mov r1, #0x100");
    asm("mul lr, r0, r1");
    asm("add lr, #0xc8");
    asm("bx lr");
}


void kinit() {
    // Initialize COM2
    bwsetspeed(COM2, 115200);
    bwsetfifo(COM2, OFF);

    // Initialize kernel constants
    task_count = 0;
    next_task = 0;
    
    id = 0;

    init_task_list();

    init_pqueue();
    
    uint *p = (uint *)0x20;
    for (int i = 0; i < 8; i++) {
        *p = (uint)unhandled_exception_handler;
        p = p + 1;
    }
    uint *handler_dest = (uint *)0x28;
    *handler_dest = (uint)enter_kernel;


    // init kernel task
    #if DEBUG_ON
    bwprintf(COM2, "\r\nKERNEL!\r\n");
    #endif
}


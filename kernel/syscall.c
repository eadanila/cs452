#include <bwio.h>

#include "arm_lib.h"
#include "syscall.h"
#include "kernel.h"
#include "pqueue.h"
#include "logging.h"
#include "task.h"

void print_regs(struct frame *fp) {

    print("fp: %x\r\n", fp);

    print("cspr: %x\r\n", fp->cspr);
    print("registers...\r\n");
    uint *p = (uint *)(&(fp->cspr) + 1);
    for (int i = 0; i < 16; i++) {
        print("%d: ", i);
        print("r%d: %x\r\n", i, *(p+i));
    }
}

// User calls syscall then:
// syscall (C) -> software_interupt (ARM) -> enter_kernel (ARM) -> handle_swi (C)

void handle_swi(int id)
{
    task t = get_task_by_id(id);
    struct frame *fp = (struct frame *)t.stack_pointer;
    
    int syscall_id = fp->r0;

//    print_regs(fp);
                
    switch(syscall_id)
    {
        case SYSCALL_YIELD:
            // All this should do is send the task to the end of the ready queue
            set_task_state(id, TASK_READY);
            push_task(id);
            break;
        case SYSCALL_EXIT:
            // TODO Reclaim task resources as well?
            set_task_state(id, TASK_ZOMBIE);
            break;
        default:
            FATAL("What is this, a syscall for ants? %d?\r\n");
        break;
    }
}

void Yield() {
    syscall(SYSCALL_YIELD);
}

void Exit()
{
    syscall(SYSCALL_EXIT);
}

void exit_handler() {
    Exit();
}

void scream(uint sp)
{
    print("scream: %x\r\n", sp);
}


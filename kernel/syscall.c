#include <bwio.h>

#include "arm_lib.h"
#include "syscall.h"
#include "kernel.h"
#include "pqueue.h"

void print_regs(struct frame *fp) {

    bwprintf(COM2, "fp: %x\r\n", fp);

    bwprintf(COM2, "cspr: %x\r\n", fp->cspr);
    bwprintf(COM2, "registers...\r\n");
    uint *p = (uint *)(&(fp->cspr) + 1);
    for (int i = 0; i < 16; i++) {
        bwprintf(COM2, "%d: ", i);
        bwprintf(COM2, "r%d: %x\r\n", i, *(p+i));
    }
}

// User calls syscall then:
// syscall (C) -> software_interupt (ARM) -> enter_kernel (ARM) -> handle_swi (C)

void handle_swi(int id)
{
    struct frame *fp = (struct frame *)tasks[id].stack_pointer;
    
    int syscall_id = fp->r0;

//    print_regs(fp);
                
    switch(syscall_id)
    {
        case SYSCALL_YIELD:
            // All this should do is send the task to the end of the ready queue
            add_task(id, tasks[id].priority);
            break;
        case SYSCALL_EXIT:
            // TODO Reclaim task resources as well?
            break;
        default:
            bwprintf(COM2, "What is this, a syscall for ants? %d?\r\n", syscall_id);
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

uint MyTid()
{
    return id;
}

uint MyParentTid()
{
    return get_task(MyTid())->p_id;
}

void scream(uint sp)
{
    bwprintf(COM2, "scream: %x\r\n", sp);
}

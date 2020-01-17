#include <bwio.h>

#include "task.h"
#include "syscall.h"
#include "arm_lib.h"

// User calls syscall then:
// syscall (C) -> software_interupt (ARM) -> enter_kernel (ARM) -> handle_swi (C)

void handle_swi(int syscall_id)
{
    switch(syscall_id)
    {
        case SYSCALL_YIELD:
            // Do stuff to yeild and switch into another task if there is one?
            // Test with 2 hard coded tasks first?
        break;
    }
}

void scream()
{
    bwputstr(COM2, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
}

TASK* schedule() {
    if (task_select == 0) {
        task_select = 1;
        return &t1;
    }
    task_select = 0;
    return &t2;
}

int syscall(int call_id) {
    struct task *old, *new;
    void *stack_pointer;
    int result = 0;

    bwprintf(COM2, "syscall\r\n");

    // disable interrupts and save state
    // maybe interrupts should be disabled sooner
//    enter_kernel_mode();
    stack_pointer = save_task_state();

    old = current_task;
    old->sp = stack_pointer;

    switch(call_id) {
    case SYSCALL_YIELD:
        new = schedule();
        current_task = new;
        restore_task_state(new->sp);
//        enter_user_mode();
        old->result = result;
        if (new->state == 0)
            new->fp();
        return new->result;
        break ;
    default: 
        return 0;
        break ;
    }
}


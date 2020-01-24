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
    DEBUG("handle_swi called");
    task t = get_task_by_id(id);
    struct frame *fp = (struct frame *)t.stack_pointer;

    int syscall_id = fp->r0;

    //    print_regs(fp);

    switch(syscall_id)
    {
        case SYSCALL_CREATE:
            DEBUG("CREATE, called by %d", id);

            fp->r0 = kcreate(fp->r1, fp->r2);

            set_task_state(id, TASK_READY);
            push_task(id);

            break;
        case SYSCALL_MY_TID:
            DEBUG("MY_TID, called by %d", id);

            fp->r0 = get_running_task();

            set_task_state(id, TASK_READY);
            push_task(id);

            break;
        case SYSCALL_PARENT_TID:
            DEBUG("PARENT_TID, called by %d", id);

            fp->r0 = get_task_by_id(get_running_task()).p_id;
            set_task_state(id, TASK_READY);

            push_task(id);

            break;
        case SYSCALL_YIELD:
            // All this should do is send the task to the end of the ready queue
            DEBUG("YIELD, called by %d", id);

            set_task_state(id, TASK_READY);
            push_task(id);

            break;
        case SYSCALL_EXIT:
            DEBUG("EXIT, called by %d", id);

            set_task_state(id, TASK_ZOMBIE);

            break;
        default:
            FATAL("What is this, a syscall for ants? %d? Called by %d", syscall_id, id);
            break;
    }
}

int kcreate(int priority, uint function) {
    DEBUG("kcreate: pri %d, fp %x", priority, function);

    if(priority < 0 || priority > MIN_PRIORITY) {
        ERROR("Invalid Priority %d for parent %d", priority, get_running_task());
        return INVALID_PRIORITY;
    }

    int p_id = get_running_task();
    int t_id = allocate_task(p_id, priority);

    if(t_id == OUT_OF_TASK_DESCRIPTORS)
    {
        WARN("Maximum number of tasks reached!");
        return t_id;
    }

    task t = get_task_by_id(t_id);
    struct frame *f = (struct frame *)(t.stack_base - 16);

    f->r13 = (uint)t.stack_base;
    f->r15 = (uint)function;
    f->r14 = (uint)exit_handler;
    f->cspr = (uint)CSPR_USER_MODE;

    set_task_stack_pointer(t_id, (uint *)t.stack_base - 16);

    push_task(t_id);

    print("Created: %d\r\n", t_id);

    return t_id;
}

int Create(int priority, void(*function)()) {
    DEBUG("Create called: pri %d, fun %x", priority, function);
    return syscall(SYSCALL_CREATE, priority, (uint)function);
}

int MyTid() {
    return syscall(SYSCALL_MY_TID, 0, 0);
}

int MyParentTid() {
    return syscall(SYSCALL_PARENT_TID, 0, 0);
}

void Yield() {
    syscall(SYSCALL_YIELD, 0, 0);
}

void Exit()
{
    syscall(SYSCALL_EXIT, 0, 0);
}

void exit_handler() {
    Exit();
}

void scream(uint sp)
{
    print("scream: %x\r\n", sp);
}


#include <bwio.h>

#include "kernel.h"
#include "arm_lib.h"
#include "syscall.h"
#include "pqueue.h"

#include "logging.h"
#include "task.h"

void unhandled_exception_handler(void);

void print_lr(uint u) {
    FATAL("We tried to access something we shouldn't here: %x\r\n", u);
}

int Create(int priority, void (*function)())
{
    if(priority < 0 || priority > MIN_PRIORITY) return INVALID_PRIORITY;

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

    push_task(id);

    print("Created: %d\r\n", id);

    return id;
}

int user_mode() {
    uint proc_state = get_cpsr();
    uint mode = (uint)proc_state & (uint)0x1F;
    return mode == CSPR_USER_MODE;
}

void panic() {
    // something bad has happened and now the kernel is in a panic
    // print panic message and return to redboot
    print("\r\n");
    print("Did you ever hear the tragedy of Darth Plagueis The Wise?\r\n");
    print("I thought not. It’s not a story the Jedi would tell you.\r\n");
    print("It’s a Sith legend. Darth Plagueis was a Dark Lord of the\r\n");
    print("Sith, so powerful and so wise he could use the Force to\r\n");
    print("influence the midichlorians to create life. He had such a\r\n");
    print("knowledge of the dark side that he could even keep the\r\n");
    print("ones he cared about from dying. The dark side of the\r\n");
    print("Force is a pathway to many abilities some consider to be\r\n");
    print("unnatural. He became so powerful the only thing he was\r\n");
    print("afraid of was losing his power, which eventually, of\r\n");
    print("course, he did. Unfortunately, he taught his apprentice\r\n");
    print("everything he knew, then his apprentice killed him in his\r\n");
    print("sleep.\r\n\r\n");

    print("Ironic. He could save others from death, but not himself.\r\n");
    print("\r\nPS: This is a panic.\r\n");

    // unregister handlers and exit to redboot
    uint *p = (uint *)0x20;
    for (int i = 0; i < 8; i++) {
        *p = 0;
        p = p + 1;
    }
    return_to_redboot();
}


void kinit() {
    // Initialize COM2
    bwsetspeed(COM2, 115200);
    bwsetfifo(COM2, OFF);

    print("\r\n");

    init_task_list();

    init_pqueue();
    
    uint *p = (uint *) IVT_BASE_ADDR;
    for (int i = 0; i < 8; i++) {
        *p = (uint)unhandled_exception_handler;
        p = p + 1;
    }
    uint *handler_dest = (uint *) IVT_SWI_ADDR;
    *handler_dest = (uint)enter_kernel;

    DEBUG("kint() finished");
}


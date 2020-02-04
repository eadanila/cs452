#include <bwio.h>

#include "kernel.h"
#include "arm_lib.h"
#include "syscall.h"
#include "pqueue.h"

#include "logging.h"

#include "user.h"

#include "timer.h"

#include "frame.h"

int main(int argc, char *argv[]) {
    kinit();

    DEBUG("Creating first task -> %x", umain);


    uint cpsr_mode = 0x13;

    int id = kcreate(3, (uint)umain);

    int sid = pop_task();
    if (id != sid)
        FATAL("Task not successfully scheduled. Got %d, expected %d", sid, id);
    set_running_task(id);

    for (;;) {
        LOG("Got ID %d from PQ %d", id, get_task_by_id(id).priority);

        if (cpsr_mode == 0x12) {
            clear_timer(TIMER_TC1);
        }

        set_task_stack_pointer(id, enter_user(get_task_stack_pointer(id)));
        cpsr_mode = get_cpsr() & 0x1F;

        if (cpsr_mode == 0x12) {
            DEBUG("IRQ CAUGHT");
            Frame *fp = (Frame *)get_task_stack_pointer(id);
            fp->r0 = 1;
            handle_swi(id);
        }
        else if (cpsr_mode == 0x13) {
            DEBUG("SWI CAUGHT");
            *((volatile uint *)0x8092008) = 0;
            handle_swi(id);
        }
        else
            panic();

        id = pop_task();
        set_running_task(id);

        if (id < 0)
            break;
    }

    print("Kernel: exiting\r\n");
    return 0;
}


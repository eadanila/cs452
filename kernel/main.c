#include <bwio.h>

#include "kernel.h"
#include "arm_lib.h"
#include "syscall.h"
#include "pqueue.h"
#include "logging.h"
#include "user.h"
#include "timer.h"
#include "frame.h"
#include "interrupt.h"
#include "idle.h"

int main(int argc, char *argv[]) {
    kinit();

    DEBUG("Creating first task -> %x", umain);


    uint cpsr_mode = 0x13;
    
    int idle_task_id = kcreate(7, (uint)idle_task);
    DEBUG("Idle task ID: %d", idle_task_id);
    int id = kcreate(3, (uint)umain);

    idle_time = 0;

    // sanity check
    int sid = pop_task();
    if (id != sid)
        FATAL("Task not successfully scheduled. Got %d, expected %d", sid, id);

    set_running_task(id);

    stop_debug_timer();
    start_debug_timer();

    for (;;) {
        LOG("Got ID %d from PQ %d", id, get_task_by_id(id).priority);

        start_time = read_debug_timer();
        set_task_stack_pointer(id, enter_user(get_task_stack_pointer(id)));
        end_time = read_debug_timer();

        if (id == idle_task_id) {
            idle_time += (end_time - start_time);
        }

        cpsr_mode = get_cpsr() & 0x1F;
        DEBUG("Processor mode: %x", cpsr_mode);

        if (cpsr_mode == 0x12) {
            DEBUG("IRQ CAUGHT");
            handle_interrupt(id);
        }
        else if (cpsr_mode == 0x13) {
            DEBUG("SWI CAUGHT");
            handle_swi(id);
            if (get_active_tasks_count() == 1)
                break;
        }

        id = pop_task();
        set_running_task(id);
    }

    kcleanup();

    print("Kernel: exiting\r\n");
    return 0;
}


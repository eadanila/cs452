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
#include "idle_printer.h"
#include "train_timing.h"

#include "frame.h"

#include "interrupt.h"

int main(int argc, char *argv[]) {
    #ifdef TIMING
    timing();
    kcleanup();

    return 0;
    #endif

    kinit();

    DEBUG("Creating first task -> %x", umain);


    uint cpsr_mode = 0x13;
    int init_done = 0;
    
    // idle_printer defines the idle_time, start_time, end_time variables
    // which are written to by the kernel and represent the time spent idle.
    // idle_task running is used solely to calculate the idle time and
    // halt the processor.
    int idle_task_id = kcreate(15, (uint)idle_task);
    DEBUG("Idle task ID: %d", idle_task_id);
    kcreate(14, (uint)idle_printer);
    int id = kcreate(13, (uint)umain);

    start_time = 0;
    end_time = 0;
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
            // If handle_swi is non_zero, a shutdown condition has occured.
            if(handle_swi(id)) break;

            if(get_active_tasks_count() > LONG_RUNNING_TASK_COUNT)
                init_done = 1;
            else if (get_active_tasks_count() <= LONG_RUNNING_TASK_COUNT && init_done) 
                break;
        }

        id = pop_task();
        set_running_task(id);
    }

    kcleanup();

    print("Kernel: exiting\n\r");
    return 0;
}


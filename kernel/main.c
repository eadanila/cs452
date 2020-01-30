#include <bwio.h>

#include "kernel.h"
#include "arm_lib.h"
#include "syscall.h"
#include "pqueue.h"

#include "logging.h"

#include "user.h"

#include "timer.h"

int main(int argc, char *argv[]) {
    kinit();

    DEBUG("Creating first task -> %x", umain);

    int id = kcreate(3, (uint)umain);

    int sid = pop_task();
    if (id != sid)
        FATAL("Task not successfully scheduled. Got %d, expected %d", sid, id);
    set_running_task(id);

    for (;;) {
        LOG("Got ID %d from PQ %d", id, get_task_by_id(id).priority);
        set_task_stack_pointer(id, enter_user(get_task_stack_pointer(id)));
        handle_swi(id);

        id = pop_task();
        set_running_task(id);

        if (id < 0)
            break;
    }

    print("Kernel: exiting\r\n");
    return 0;
}


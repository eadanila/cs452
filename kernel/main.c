#include <bwio.h>

#include "kernel.h"
#include "arm_lib.h"
#include "syscall.h"
#include "pqueue.h"

#include "logging.h"

void test_task(void)
{
    bwprintf(COM2, "Task ID: %d, Parent ID: %d\r\n", MyTid(), MyParentTid());
    Yield();
    bwprintf(COM2, "Task ID: %d, Parent ID: %d\r\n", MyTid(), MyParentTid());
}

void first_task(void)
{
    // 3 is lower priority than 1
    Create(3, test_task);
    Create(3, test_task);

    Create(1, test_task);
    Create(1, test_task);

    bwprintf(COM2, "FirstUserTask: exiting\r\n");
}

int main(int argc, char *argv[]) {
    kinit();
    id = Create(2, first_task);
    assert(id == next_scheduled_task());

    for (;;) {
        DEBUG("Got ID %d from PQ %d", id, tasks[id].priority);
        tasks[id].stack_pointer = enter_user(tasks[id].stack_pointer);
        handle_swi(id);
        id = next_scheduled_task();

        if (id < 1)
            break;
    }
    bwprintf(COM2, "Kernel: exiting");
    return 0;
}


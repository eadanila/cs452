#include <bwio.h>

#include "kernel.h"
#include "arm_lib.h"
#include "syscall.h"
#include "pqueue.h"

#include "logging.h"

void test_task(void)
{
    print("Task ID: %d, Parent ID: %d\r\n", MyTid(), MyParentTid());
    Yield();
    print("Task ID: %d, Parent ID: %d\r\n", MyTid(), MyParentTid());
    Exit();
}

void first_task(void)
{
    // 3 is lower priority than 1
    DEBUG("");
    Create(3, test_task);
    Create(3, test_task);

    Create(1, test_task);
    Create(1, test_task);

    print("FirstUserTask: exiting\r\n");

    Exit();
}

int main(int argc, char *argv[]) {
    kinit();
    DEBUG("Creating first task -> %x", first_task);
    int id = kcreate(2, (uint)first_task);

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


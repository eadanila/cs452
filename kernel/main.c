#include <bwio.h>

#include "kernel.h"
#include "arm_lib.h"
#include "syscall.h"
#include "pqueue.h"

void task_1(void) {
    while(1) {
        bwprintf(COM2, "task 1\r\n");
        Yield();
    }
}

void task_2(void) {
    while(1) {
        bwprintf(COM2, "task 2\r\n");
        Yield();
    }
}

void user_task(void) {
    Create(0, task_1);
    Create(0, task_2);
    while(1) {
        bwprintf(COM2, "hello, world\r\n");
        Yield();
    }
}

void test_task(void)
{
    bwprintf(COM2, "Task ID: %d, Parent ID: %d\r\n", MyTid(), MyParentTid());
    Yield();
    bwprintf(COM2, "Task ID: %d, Parent ID: %d\r\n", MyTid(), MyParentTid());
    Exit();
}

void first_task(void)
{
    // 3 is lower priority than 1
    Create(3, test_task);
    Create(3, test_task);

    Create(1, test_task);
    Create(1, test_task);

    bwprintf(COM2, "FirstUserTask: exiting\r\n");
    Exit();
}

int main(int argc, char *argv[]) {
    kinit();
    int id = Create(2, first_task);
    int priority = 0;
    for (int i = 0; i < 3*5; i++) {
        tasks[id].stack_pointer = enter_user(tasks[id].stack_pointer);
        handle_swi(tasks[id].stack_pointer);

        priority = get_current_priority();
        if (priority == -1)
            break;

        id = front_task(priority);
        #if DEBUG_ON
        bwprintf(COM2, "Got ID %d from PQ %d\r\n", id, priority);
        #endif

        if (id == -1)
            break;
    }
    bwprintf(COM2, "Kernel: exiting");
    return 0;
}


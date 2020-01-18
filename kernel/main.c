#include <bwio.h>

#include "kernel.h"
#include "arm_lib.h"
#include "syscall.h"

void task_1(void) {
    bwprintf(COM2, "task 1\r\n");
}

void task_2(void) {
    bwprintf(COM2, "task 2\r\n");
}

void user_task(void) {
    bwprintf(COM2, "hello, world\r\n");
    //int t1 = task_init(1, &task_1);
    //int t2 = task_init(1, &task_2);
}

int main(int argc, char *argv[]) {
    kinit();
    int id = task_init(0, user_task);
    bwprintf(COM2, "aaaaaaaaa\r\n");
    for (;;) {
        tasks[id].stack_pointer = enter_user(tasks[id].stack_pointer);
        handle_swi(tasks[id].stack_pointer);

        bwprintf(COM2, "done\r\n");
        return 0;
    }
    return 0;
}


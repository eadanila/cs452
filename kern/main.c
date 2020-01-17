#include <bwio.h>
#include <ts7200.h>
#include <limits.h>

#include "syscall.h"
#include "task.h"

TASK ktask;

void Yield() {
    bwprintf(COM2, "yeild: task %d\r\n", current_task->t_id);
    syscall(SYSCALL_YIELD);
    bwprintf(COM2, "resume: task %d\r\n", current_task->t_id);
}

void t1_f(void) {
    bwprintf(COM2, "task 1\r\n");
    Yield();
    bwprintf(COM2, "task 1\r\n");
}

void t2_f(void) {
    bwprintf(COM2, "task 2\r\n");
    Yield();
    bwprintf(COM2, "task 2\r\n");
}

void initialize(void) {
    bwsetspeed(COM2, 115200);
    bwsetfifo(COM2, OFF);

    ktask.t_id = 0;
    ktask.sp = (void *)0x01000000;
//    ktask.fp = &kernel;

    t1.t_id = 1;
    t1.sp = (void *)0x20000000;
    t1.fp = &t1_f;
    t1.state = 0;
    t2.t_id = 2;
    t2.sp = (void *)(0x20000000 - 0x01000000);
    t2.fp = &t2_f;
    t2.state = 0
}

int main(int argc, char *argv[]) {
    initialize();
    current_task = &t1;
    task_select = 1;
    bwprintf(COM2, "Kernel init complete\r\n");
    current_task->fp();
    return 0;
}

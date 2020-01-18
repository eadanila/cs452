#include <bwio.h>

#include "kernel.h"
#include "arm_lib.h"
#include "syscall.h"

void fuck(void);

void print_lr(uint u) {
    bwprintf(COM2, "shit's broke: %x\r\n", u);
    while (1);
}

// lowest address
void *user_stack = (void *)0x2000000;

// excluding kernel
int task_count = 0;

int next_t_id(void) {
    task_count = task_count + 1;
    return task_count;
}

int task_init(int p_id, void (*f)(void)) {
    int id = next_t_id();
    uint *stack_base = (uint *)0x0200000;
    bwprintf(COM2, "deref1\r\n");

    struct frame *fr = (struct frame *)(stack_base - 16);
    bwprintf(COM2, "deref2\r\n");
    tasks[id].t_id = id;
    bwprintf(COM2, "deref3\r\n");
    tasks[id].p_id = p_id;
    bwprintf(COM2, "deref4\r\n");
    tasks[id].stack_base = stack_base;
    bwprintf(COM2, "deref5\r\n");
    tasks[id].stack_pointer = stack_base - 16;
    bwprintf(COM2, "deref6\r\n");
    tasks[id].pc = f;

    bwprintf(COM2, "deref7\r\n");

    uint *p = ((uint *)stack_base) - 16;

    for (int i = 0; i < 17; i++) {
        *(p+i) = 0;
    }

    fr->r15 = (uint)f;
    fr->r13 = (uint)stack_base;
    fr->r14 = (uint)exit_handler;
    fr->cspr = (uint)0x10;

    for (int i = 0; i < 17; i++) {
        bwprintf(COM2, "i:%d,&i:%x\r\n", i, *(p + i));
    }

    bwprintf(COM2, "\r\nDone task_init\r\n");

    return id;
}

void kinit() {
    // init COM2
    bwsetspeed(COM2, 115200);
    bwsetfifo(COM2, OFF);

    task_count = 0;
    uint *p = (uint *)0x20;
    for (int i = 0; i < 8; i++) {
        *p = (uint)fuck;
        p = p + 1;
    }
    uint *handler_dest = (uint *)0x28;
    *handler_dest = (uint)enter_kernel;


    // init kernel task
    tasks[0].t_id = 0;
    tasks[0].p_id = 0;
    tasks[0].stack_pointer = (uint *)0x01000000;
    tasks[0].pc = 0x0; // what should this be for the kernel?
    bwprintf(COM2, "\r\nKERNEL!\r\n");
}


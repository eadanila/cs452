#ifndef KERNEL_H
#define KERNEL_H

typedef unsigned int uint;

struct __attribute__((__packed__)) frame {
    uint cspr;
    uint r0;
    uint r1;
    uint r2;
    uint r3;
    uint r4;
    uint r5;
    uint r6;
    uint r7;
    uint r8;
    uint r9;
    uint r10;
    uint r11;
    uint r12;
    uint r13;
    uint r14;
    uint r15;
};
typedef struct frame FRAME;

struct __attribute__((__packed__)) task {
    int t_id;
    int p_id;
    uint *stack_base;
    uint *stack_pointer;
    void (*pc)(void);
};
struct task TASK;

struct task tasks[5];

int task_init(int p_id, void (*f)(void));

void kinit(void);

#endif

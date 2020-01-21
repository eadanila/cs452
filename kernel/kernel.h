#ifndef KERNEL_H
#define KERNEL_H

#include "pqueue.h"
#include "constants.h"

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
    int is_valid;
    int t_id;
    int p_id;
    uint *stack_base;
    uint *stack_pointer;
    void (*pc)(void);
    int next;
};
typedef struct task TASK;

void schedule(void);

int task_count;
int next_task;
TASK tasks[MAX_TASKS_ALLOWED];
PQUEUE task_schedule[MIN_PRIORITY];
TASK* get_task(uint id);

int Create(int priority, void (*function)());

void kinit(void);

int user_mode(void);
void panic(void);

#endif

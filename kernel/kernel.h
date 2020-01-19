#ifndef KERNEL_H
#define KERNEL_H

typedef unsigned int uint;

#define MEMORY_END 0x2000000
#define MEMORY_START 0x0100000
#define TASK_MEMORY_SIZE 0x04000 // 16 KiB
#define MAXIMUM_TASKS_ALLOWED 1024

#define CSPR_USER_MODE 0x10

// Error codes
#define OUT_OF_TASK_DESCRIPTORS -2

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
};
typedef struct task TASK;

int task_count;
int next_task;
struct task tasks[MAXIMUM_TASKS_ALLOWED];

int Create(int priority, void (*function)());

void kinit(void);

#endif

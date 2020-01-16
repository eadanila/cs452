#include "kernel.h"
#include "syscall.h"
#include "arm_lib.h"

#define MEMORY_START   0x0100000
#define MEMORY_END     0x2000000

#define TASK1_STACK    0x0200000
#define TASK2_STACK    0x0400000

struct task
{
    int tid;
    int* stack;
};

struct task tasks[2];
int task_cnt;

void init_kernel()
{
    // Set the function that software interupts call
    int * handler_destination = 0x28;
    *handler_destination =  (int*) enter_kernel;

    struct task task1;
    struct task task2;
    tasks[0] = task1;
    tasks[1] = task2;

    tasks[0].stack = TASK1_STACK;
    tasks[1].stack = TASK2_STACK;

    task_cnt = 0;
}

void task1()
{
    Yield();
}

void task2()
{

}

int Create(int priority, void (*function)())
{
    // Currently just 2 tasks being created in initialization and just activates one of them
    // Currently want to test if we can enter a user mode and come back without catastrophically failing

    enter_user(TASK1_STACK);
}

void Yield()
{
    syscall(SYSCALL_YIELD);
}
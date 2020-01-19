#ifndef ARM_LIB_H
#define ARM_LIB_H

#include "kernel.h"

extern uint *enter_kernel();
extern uint *enter_user(uint *task_stack);
extern int syscall(int syscall_id);
extern int init_task(void *stack_ptr, void (*f)(void)); // returns new top of stack

#endif

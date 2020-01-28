#ifndef ARM_LIB_H
#define ARM_LIB_H

#include "kernel.h"

extern uint *enter_kernel();
extern uint *enter_user(uint *task_stack);
extern int syscall(int syscall_id, int arg1, int arg2, int arg3, int arg4, int arg5);

extern uint get_cpsr(void);
extern void return_to_redboot(void);

#endif


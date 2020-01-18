#define SYSCALL_YIELD 1
#define SYSCALL_EXIT 2

#include "kernel.h"

void handle_swi(uint *stack_pointer);

void Yield();
int syscall(int id);

// Put "b scream" anywhere in assembly for basic debugging
void scream();

void exit_handler();

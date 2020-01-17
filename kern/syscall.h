#define SYSCALL_YIELD 1

#include "arm_lib.h"

int syscall(int id);

// Put "b scream" anywhere in assembly for basic debugging
void scream();

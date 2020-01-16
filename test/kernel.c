#include "kernel.h"
#include "syscall.h"

int Create(int priority, void (*function)())
{
    
}

void Yield()
{
    syscall(SYSCALL_YIELD);
}
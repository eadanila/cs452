#ifndef SYSCALL_H
#define SYSCALL_H

#define SYSCALL_YIELD 1
#define SYSCALL_EXIT 2
#define SYSCALL_CREATE 3
#define SYSCALL_MY_TID 4
#define SYSCALL_PARENT_TID 5

#include "kernel.h"

void handle_swi(int id);

int kcreate(int priority, uint function);

int Create(int priority, void (*function)());
int MyTid(void);
int MyParentTid(void);
void Yield(void);
void Exit(void);

// Put "b scream" anywhere in assembly for basic debugging
void scream(uint sp);

void exit_handler();

#endif


#ifndef SYSCALL_H
#define SYSCALL_H

#define SYSCALL_YIELD 1
#define SYSCALL_EXIT 2

#include "kernel.h"

void handle_swi(int id);

void Yield();
void Exit();
uint MyTid();
uint MyParentTid();

// Put "b scream" anywhere in assembly for basic debugging
void scream(uint sp);

void exit_handler();

#endif

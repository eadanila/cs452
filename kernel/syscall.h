#ifndef SYSCALL_H
#define SYSCALL_H

#define SYSCALL_YIELD 1
#define SYSCALL_EXIT 2
#define SYSCALL_CREATE 3
#define SYSCALL_MY_TID 4
#define SYSCALL_PARENT_TID 5

#define SYSCALL_SEND 6
#define SYSCALL_RECEIVE 7
#define SYSCALL_REPLY 8

#define SYSCALL_AWAIT 9

#include "kernel.h"

void handle_swi(int caller);

int kcreate(int priority, uint function);

int Create(int priority, void(*function)());
int MyTid(void);
int MyParentTid(void);
void Yield(void);
void Exit(void);

int Send(int tid, const char *msg, int msglen, char *reply, int rplen);
int Receive(int *tid, char *msg, int msglen);
int Reply(int tid, const char *reply, int rplen);

int AwaitEvent(int eventid);

// Put "b scream" anywhere in assembly for basic debugging
void scream(uint sp);

void exit_handler();

#endif


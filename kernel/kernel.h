#ifndef KERNEL_H
#define KERNEL_H

#include "task.h"
#include "pqueue.h"
#include "constants.h"

int kcreate(int priority, uint function);
void kcopymessage(int dest_id, int src_id);
void kcopyreply(int dest_id, int src_id);

void kinit(void);

int user_mode(void);
void panic(void);

#endif


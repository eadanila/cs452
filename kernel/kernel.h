#ifndef KERNEL_H
#define KERNEL_H

#include "task.h"
#include "pqueue.h"
#include "constants.h"

void kinit(void);

int user_mode(void);
void panic(void);

#endif


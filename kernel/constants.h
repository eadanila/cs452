#ifndef CONSTANTS_H
#define CONSTANTSL_H

typedef unsigned int uint;

#define DEBUG_ON 0

#define MEMORY_END 0x2000000
#define MEMORY_START 0x0100000
#define TASK_MEMORY_SIZE 0x04000 // 16 KiB
#define MAX_TASKS_ALLOWED 1024
#define MIN_PRIORITY 7

#define CSPR_USER_MODE 0x10

// Error codes
#define OUT_OF_TASK_DESCRIPTORS -2


#endif
#ifndef CONSTANTS_H
#define CONSTANTS_H

#define KNAME "PlagueOS" // from Darth Plagueis

typedef unsigned int uint;

#define DEBUG_ON 0

#define MEMORY_END 0x1FE0000 // k_stack base - 128KiB
#define MEMORY_START 0x0200000
#define TASK_MEMORY_SIZE 0x20000 // 128 KiB
#define MAX_TASKS_ALLOWED 128
#define MIN_PRIORITY 15
#define PRIORITY_COUNT (MIN_PRIORITY + 1)

#define CSPR_USER_MODE 0x10

#define IVT_BASE_ADDR (volatile unsigned int *)0x20
#define IVT_SWI_ADDR (volatile unsigned int *)0x28
#define IVT_IRQ_ADDR (volatile unsigned int *)0x38

#define SYS_SW_LOCK_ADDR (volatile unsigned int *)0x809300C0
#define SW_HALT_ENABLE_ADDR (volatile unsigned int *)0x80930080
#define HALT_MODE_ADDR (volatile unsigned int *)0x80930008

// Error codes
#define OUT_OF_TASK_DESCRIPTORS -2
#define INVALID_PRIORITY -1

#define NULL 0
#define NOARG 0

#endif


#include "syscall.h"
#include <bwio.h>
#include "arm_lib.h"

// User calls syscall then:
// syscall (C) -> software_interupt (ARM) -> enter_kernel (ARM) -> handle_swi (C)

void handle_swi(int syscall_id)
{
    switch(syscall_id)
    {
        case SYSCALL_YIELD:
            // Do stuff to yeild and switch into another task if there is one?
            // Test with 2 hard coded tasks first?
        break;
    }
}

int syscall(int id)
{
    software_interupt(id);
}

void scream()
{
    bwputstr(COM2, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
}

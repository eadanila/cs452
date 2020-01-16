#include "syscall.h"
#include <bwio.h>

extern int enter_kernel();
extern void enter_user(int task_id);
extern void software_interupt(int syscall_id);

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

void init_kernel()
{
    // Set the function that software interupts call
    int * handler_destination = 0x28;
    *handler_destination =  (int*) enter_kernel;


}

int syscall(int id)
{
    software_interupt(id);
}

void scream()
{
    bwputstr(COM2, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
}
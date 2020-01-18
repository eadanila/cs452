#include <bwio.h>

#include "arm_lib.h"
#include "syscall.h"

void print_regs(struct frame *fp) {

    bwprintf(COM2, "fp: %x\r\n", fp);

    bwprintf(COM2, "cspr: %x\r\n", fp->cspr);
    bwprintf(COM2, "registers...\r\n");
    uint *p = &(fp->r0);
    for (int i = 0; i < 16; i++) {
        bwprintf(COM2, "%d: ", i);
        bwprintf(COM2, "r%d: %x\r\n", i, *(p+i));
    }
}

// User calls syscall then:
// syscall (C) -> software_interupt (ARM) -> enter_kernel (ARM) -> handle_swi (C)

void handle_swi(uint stack_pointer)
{
    struct frame *fp = (struct frame *)stack_pointer;
    
    int syscall_id = fp->r0;

    print_regs(fp);
                
    switch(syscall_id)
    {
        case SYSCALL_YIELD:
            // Do stuff to yeild and switch into another task if there is one?
            // Test with 2 hard coded tasks first?
            break;
        case SYSCALL_EXIT:
            bwprintf(COM2, "We made it to SYSCALL_EXIT\r\n");
            break;
        default:
            bwprintf(COM2, "What is syscall: %d?\r\n", syscall_id);
            while(1);
        break;
    }
}

void exit_handler(int exit_code) {
    bwprintf(COM2, "exit handler\r\n");
    syscall(SYSCALL_EXIT);
}

void scream()
{
    bwputstr(COM2, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
}

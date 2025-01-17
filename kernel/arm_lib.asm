.global arg_return_test
.global hardware_enter_kernel
.global software_enter_kernel
.global enter_user
.global syscall
.global unhandled_exception_handler

.global enable_cache

.global get_cpsr
.global return_to_redboot

arg_return_test:
    //mov r0, #23
    mov r1, #0x0100000
    //mov r4, r0
    str r0, [r1]
    mov r0, #23

    bx lr

syscall:
    // r0 contains the syscall code
    swi #0

    // after handling syscall return here
    bx lr

// TODO Add syscall code that causes a SWI and triggers enter kernel.

// This is where hardware interrupts will enter the kernel
hardware_enter_kernel:
    // Save r0 onto the kernel stack
    push {r0}

    // Go into system mode to gain access to banked r13 (user's sp) and
    // copy it into r0
    msr cpsr, #0b11011111
    mov r0, r13

    // Return to IRQ mode. The kernel checks the processor mode to know
    // if it is handling a software or a hardware interrupt.
    msr cpsr, #0b11010010

    // Account for user's PC advancing before harware interrupt is thrown
    sub lr, lr, #4

    b finish_enter_kernel

// This is where software interrupts will enter the kernel
software_enter_kernel:
    // Save r0 onto the kernel stack
    push {r0}

    // Go into system mode to gain access to banked r13 (user's sp) and
    // copy it into r0
    msr cpsr, #0b11011111 
    mov r0, r13

    // Return to supervisor mode to continue handling the software
    // interrupt
    msr cpsr, #0b11010011

    b finish_enter_kernel
    
// BOTH hardware and software interrupts end up here
// Save the user's register onto the user stack, restore kernel 
// registers and return to the kernel C loop.
finish_enter_kernel:
    // Push r1-r14 onto user stack
    stmdb r0, {r1-r14}^ // when I make stmdb decrement r0 itself (use `r0!`), arm-none-eabi-as complains that it is UNPREDICTABLE
    sub r0, r0, #56 // 14x4=56

    // Put the original r0 and spsr onto user stack
    pop {r2}
    mrs r1, spsr
    stmdb r0!, {r1,r2}

    // Copy return address (lr or pc of user) onto user stack
    stmdb r0!, {lr}

    // Pop r1-r14, r0 is the return address and will contain
    // the stack pointer of the user that just exited to kernel
    ldmia sp!, {r4-r11,r14}

    // return to the kernel C loop
    bx lr

    // we shouldn't get here because enter_user should switch our address space
    b print_lr
    b panic


// save kernel registers on kernel stack
// switch to user mode
// pop user registers from user stack
// upon pop, pc will point to wherever the user left off (entry point if new task)
enter_user:
    // stack to return to is provided as argument in r0
    
    // push kernel registers onto kernel stack
    stmdb sp!, {r4-r11,r14}

    // Save the current mode SP into r3
    // This will be used to set the other mode's sp
    mov r3, sp

    // get the processor mode
    mrs r2, cpsr
    and r2, #0x1F // first 5 bits
    cmp r2, #0x13 // supervisor mode
    beq supervisor_mode_enter_user

    cmp r2, #0x12 // IRQ mode
    beq irq_mode_enter_user

    // I got into enter_user from neither supervisor or IRQ
    // that's not right
    // guess I'll die.jpg
    bl print_lr
    b panic

// we were in supervisor mode
// switch to IRQ mode and set the sp to the one saved in r3 from
// supervisor
supervisor_mode_enter_user:
    msr cpsr, #0b11010010
    mov sp, r3

    b finish_enter_user

// we were in IRQ mode
// switch to supervisor mode and set the sp to the one saved from IRQ
irq_mode_enter_user:
    msr cpsr, #0b11010011
    mov sp, r3
    
    b finish_enter_user

// set user mode and load registers, that'll get us in the user task
finish_enter_user:
    // get the user's stack-saved lr using the user stack pointer in r0
    ldmia r0!, {lr}

    // get the user's stack-saved spsr and set the current mode's spsr
    // this will be restored by movs
    ldmia r0!, {r1}
    msr spsr, r1

    // restore user mode's r0-r14
    ldmia r0, {r0-r14}^

    // movs copies the spsr of the current mode into the cpsr if the
    // destination register is r15, as we do here
    movs pc, lr

    // should not get here
    // someone done goofed
    bl print_lr
    b panic


enable_cache:
    // get cp15,r1 into r0
    // cp15: system control coprocessor
    //   r1: control register
    mrc p15, 0, r0, c1, c0, 0
    // enable bits
    //  26 -> L2 cache
    //  12 -> ICache
    //   2 -> DCache
    //   0 -> MMU
    orr r0, #(1 << 26) 
    orr r0, #(1 << 12)
    orr r0, #(1 << 2)
    orr r0, #1
    // copy r0 into cp15,r1
    mcr p15, 0, r0, c7, c7, 0
    mcr p15, 0, r0, c1, c0, 0
    bx lr


unhandled_exception_handler:
    mov r0, lr
    b print_lr
    b panic


get_cpsr:
    mrs r0, cpsr
    bx lr


return_to_redboot:
// The redboot return address is 0x174C8, but due to restrictions on
// mov this cannot be loaded directly. Instead we load 0x174 into r0
// and shift it twice by multiplying with 0x100 into lr, then add 0xC8
    mov r0, #0x174
    mov r1, #0x100
    mul lr, r0, r1
    add lr, #0xc8
    bx lr























// The Shadow Realm

//sub sp, sp, #60
//str r0, [sp]
//str r1, [sp, #4]
//str r2, [sp, #8]
//str r3, [sp, #12]
//str r4, [sp, #16]
//str r5, [sp, #20]
//str r6, [sp, #24]
//str r7, [sp, #28]
//str r8, [sp, #32]
//str r9, [sp, #36]
//str r10, [sp, #40]
//str r11, [sp, #44]
//str r12, [sp, #48]
//str r13, [sp, #52]
//str r14, [sp, #56]


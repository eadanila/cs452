.global arg_return_test
.global save_task_state
.global restore_task_state
.global software_interupt
.global init_task

arg_return_test:
    @mov r0, #23
    mov r1, #0x0100000
    @mov r4, r0
    str r0, [r1]
    mov r0, #23

    bx lr

software_interupt:
    @ r0 contains the syscall code
    swi #0

@ TODO Add syscall code that causes a SWI and triggers enter kernel.

save_task_state:
    push {r0}

    @ switch to system processor mode
    cps #0b11111 

    mov r0, sp

    @ switch back into supervisor processor mode
    cps #0b10011

    @ Copy return address (lr or pc of user) onto user stack
    sub r0, r0, #4
    str lr, [r0]

    @ Push r1-r14 onto user stack
    stmdb r0, {r1-r14}^

    @ Put the original r0 onto user stack
    pop {r1}
    sub r0, r0, #4
    str r1, [r0]

    @ Save spsr
    mrs r1, spsr
    sub r0, r0, #4
    str r1, [r0]

    @ Pop r1-r14, r0 is the return address and will contain
    @ the stack pointer of the user that just exited to kernel
    ldmia sp, {r1-r14}

    @ b scream

    @ Once inside of kernel, enter handler
    b handle_swi


restore_task_state:
    @ stack to return to is provided as argument in r0
    
    @ push kernel registers onto kernel stack
    stmdb sp, {r1-r14}

    @ load spsr into r1
    ldr r1, [r0]
    add r0, r0, #4

    @ switch to user mode and load in user registers!
    msr cpsr_all, r1
    ldmia r0, {r0-r15}

@ Could have this instead of intializing a placeholder set of registers on stack.
@enter_user_first_time:

@ This will need to initialize a stack frame such that it accepts an enter user call onto it
@ See the header for it in arm_lib.h for parameters etc...
init_task:
    sub r0, r0, #64
    mov r3, #0






























@ The Shadow Realm

@sub sp, sp, #60
@str r0, [sp]
@str r1, [sp, #4]
@str r2, [sp, #8]
@str r3, [sp, #12]
@str r4, [sp, #16]
@str r5, [sp, #20]
@str r6, [sp, #24]
@str r7, [sp, #28]
@str r8, [sp, #32]
@str r9, [sp, #36]
@str r10, [sp, #40]
@str r11, [sp, #44]
@str r12, [sp, #48]
@str r13, [sp, #52]
@str r14, [sp, #56]

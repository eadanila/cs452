#include <bwio.h>

#include "arm_lib.h"
#include "syscall.h"
#include "kernel.h"
#include "pqueue.h"
#include "logging.h"
#include "task.h"
#include "frame.h"
#include "message_queue.h"

void print_regs(Frame *fp) {

    print("fp: %x\r\n", fp);

    print("cspr: %x\r\n", fp->cspr);
    print("registers...\r\n");
    uint *p = (uint *)(&(fp->cspr) + 1);
    for (int i = 0; i < 16; i++) {
        print("%d: ", i);
        print("r%d: %x\r\n", i, *(p+i));
    }
}


// User calls syscall then:
// syscall (C) -> software_interupt (ARM) -> enter_kernel (ARM) -> handle_swi (C)

void handle_swi(int caller)
{
    DEBUG("handle_swi called");
    assert(is_valid_task(caller));

    Frame *fp = (Frame *)get_task_stack_pointer(caller);
    int syscall_id = fp->r0;

    switch(syscall_id)
    {
        case SYSCALL_CREATE:
            DEBUG("CREATE, called by %d", caller);

            fp->r0 = kcreate(fp->r1, fp->r2);

            set_task_state(caller, TASK_READY);
            push_task(caller);

            break;
        case SYSCALL_MY_TID:
            DEBUG("MY_TID, called by %d", caller);

            fp->r0 = get_running_task();

            set_task_state(caller, TASK_READY);
            push_task(caller);

            break;
        case SYSCALL_PARENT_TID:
            DEBUG("PARENT_TID, called by %d", caller);

            fp->r0 = get_task_by_id(get_running_task()).p_id;

            set_task_state(caller, TASK_READY);
            push_task(caller);

            break;
        case SYSCALL_YIELD:
            // All this should do is send the Task to the end of the ready queue
            DEBUG("YIELD, called by %d", caller);

            set_task_state(caller, TASK_READY);
            push_task(caller);

            break;
        case SYSCALL_EXIT:
            DEBUG("EXIT, called by %d", caller);

            set_task_state(caller, TASK_ZOMBIE);

            //            clear_message_queue(caller);
            while (peek_message(caller) > -1) {
                assert(is_valid_task(peek_message(caller)));
                int sender = pop_message(caller);
                Frame *sender_frame = (Frame *)get_task_stack_pointer(sender);
                sender_frame->r0 = -2;
                set_task_state(sender, TASK_READY);
            }

            break;
        case SYSCALL_SEND:
            DEBUG("SEND, called by %d", caller);

            DEBUG("arg1: %x, arg2: %x, arg3: %x, arg4: %x, arg5: %x", fp->r1, fp->r2, fp->r3, fp->stk0, fp->stk1);

            // check if target is in receive state
            // if so, process send
            // else block on their message queue
            
            set_task_state(caller, TASK_SEND_WAIT);

            if (!is_valid_task(fp->r1) || get_task_state(fp->r1) == TASK_ZOMBIE) {
                fp->r0 = -1;
            }

            if (get_task_state(fp->r1) == TASK_RECV_WAIT) {
                // kmessagesend should set the return and state for dest
                kcopymessage(fp->r1, caller);
            } else {
                push_message(fp->r1, caller);
            }

            break;
        case SYSCALL_RECEIVE:
            DEBUG("RECEIVE, called by %d", caller);

            set_task_state(caller, TASK_RECV_WAIT);

            if (peek_message(caller) > -1) {
                kcopymessage(caller, pop_message(caller));
            }

            break;
        case SYSCALL_REPLY:
            DEBUG("REPLY, called by %d", caller);

            set_task_state(caller, TASK_SEND_WAIT);

            if (!is_valid_task(fp->r1) || get_task_state(fp->r1) == TASK_ZOMBIE) {
                fp->r0 = -1;
            }

            if (get_task_state(fp->r1) != TASK_RPLY_WAIT) {
                fp->r0 = -2;
            }

            kcopyreply(fp->r1, caller);

            // if both of same priority, sender runs first
            set_task_state(caller, TASK_READY);
            push_task(caller);
            
            break;
        default:
            FATAL("What is this, a syscall for ants? %d? Called by %d", syscall_id, caller);
            break;
    }
}

int Create(int priority, void(*function)()) {
    DEBUG("Create called: pri %d, fun %x", priority, function);
    return syscall(SYSCALL_CREATE, priority, (uint)function, 0, 0, 0);
}

int MyTid(void) {
    return syscall(SYSCALL_MY_TID, 0, 0, 0, 0, 0);
}

int MyParentTid(void) {
    return syscall(SYSCALL_PARENT_TID, 0, 0, 0, 0, 0);
}

void Yield(void) {
    syscall(SYSCALL_YIELD, 0, 0, 0, 0, 0);
}

void Exit(void)
{
    syscall(SYSCALL_EXIT, 0, 0, 0, 0, 0);
}

int Send(int tid, const char *msg, int msglen, char *reply, int rplen) {
    return syscall(SYSCALL_SEND, (int)tid, (int)msg, (int)msglen, (int)reply, (int)rplen);
}

int Receive(int *tid, char *msg, int msglen) {
    return syscall(SYSCALL_RECEIVE, (int)tid, (int)msg, (int)msglen, 0, 0);
}

int Reply(int tid, const char *reply, int rplen) {
    return syscall(SYSCALL_REPLY, (int)tid, (int)reply, (int)rplen, 0, 0);
}

void exit_handler() {
    Exit();
}

void scream(uint sp)
{
    print("scream: %x\r\n", sp);
}


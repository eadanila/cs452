#include <bwio.h>

#include "kernel.h"
#include "arm_lib.h"
#include "syscall.h"
#include "pqueue.h"
#include "message_queue.h"

#include "logging.h"
#include "task.h"
#include "frame.h"

#include "timer.h"

void unhandled_exception_handler(void);

void print_lr(uint u) {
    FATAL("We tried to access something we shouldn't here: %x", u);
}

int user_mode(void) {
    uint proc_state = get_cpsr();
    uint mode = (uint)proc_state & (uint)0x1F;
    return mode == CSPR_USER_MODE;
}

void panic(void) {
    // something bad has happened and now the kernel is in a panic
    // print panic message and return to redboot
    print("\r\n");
    print("Did you ever hear the tragedy of Darth Plagueis The Wise?\r\n");
    print("I thought not. It’s not a story the Jedi would tell you.\r\n");
    print("It’s a Sith legend. Darth Plagueis was a Dark Lord of the\r\n");
    print("Sith, so powerful and so wise he could use the Force to\r\n");
    print("influence the midichlorians to create life. He had such a\r\n");
    print("knowledge of the dark side that he could even keep the\r\n");
    print("ones he cared about from dying. The dark side of the\r\n");
    print("Force is a pathway to many abilities some consider to be\r\n");
    print("unnatural. He became so powerful the only thing he was\r\n");
    print("afraid of was losing his power, which eventually, of\r\n");
    print("course, he did. Unfortunately, he taught his apprentice\r\n");
    print("everything he knew, then his apprentice killed him in his\r\n");
    print("sleep.\r\n\r\n");

    print("Ironic. He could save others from death, but not himself.\r\n");
    print("\r\nPS: This is a panic.\r\n");

    // return to redboot
    return_to_redboot();
}

int kcreate(int priority, uint function) {
    DEBUG("kcreate: pri %d, fp %x", priority, function);

    if(priority < 0 || priority > MIN_PRIORITY) {
        ERROR("Invalid Priority %d for parent %d", priority, get_running_task());
        return INVALID_PRIORITY;
    }

    int p_id = get_running_task();
    int t_id = allocate_task(p_id, priority);

    if(t_id == OUT_OF_TASK_DESCRIPTORS)
    {
        WARN("Maximum number of Tasks reached!");
        return OUT_OF_TASK_DESCRIPTORS;
    }

    Task t = get_task_by_id(t_id);
    struct frame *f = (struct frame *)(t.stack_base - 16);

    f->r13 = (uint)t.stack_base;
    f->r15 = (uint)function;
    f->r14 = (uint)exit_handler;
    f->cspr = (uint)CSPR_USER_MODE;

    set_task_stack_pointer(t_id, (uint *)t.stack_base - 16);

    push_task(t_id);

    return t_id;
}

int copy(char *dest, int destlen, const char *src, int srclen) {
    int maxcpy = destlen;
    if (destlen > srclen)
        maxcpy = srclen;

    for (int i = 0; i < maxcpy; i++) {
        dest[i] = src[i];
    }

    return maxcpy;
}

void kcopymessage(int dest_id, int src_id) {
    assert(is_valid_task(dest_id));
    assert(is_valid_task(src_id));
    assert(get_task_state(dest_id) == TASK_RECV_WAIT);
    assert(get_task_state(src_id) == TASK_SEND_WAIT);

    Frame *dest = (Frame *)get_task_stack_pointer(dest_id);
    Frame *src = (Frame *)get_task_stack_pointer(src_id);

    const char *msg = (const char *)src->r2;
    int msglen = (int)src->r3;

    int *tid = (int *)dest->r1;
    char *buf = (char *)dest->r2;
    int buflen = (int)dest->r3;

    *tid = src_id;
    dest->r0 = copy(buf, buflen, msg, msglen);

    set_task_state(src_id, TASK_RPLY_WAIT);
    set_task_state(dest_id, TASK_READY);
    push_task(dest_id);
}

void kcopyreply(int dest_id, int src_id) {
    assert(is_valid_task(dest_id));
    assert(is_valid_task(src_id));
    assert(get_task_state(dest_id) == TASK_RPLY_WAIT);
    assert(get_task_state(src_id) == TASK_SEND_WAIT);

    Frame *dest = (Frame *)get_task_stack_pointer(dest_id);
    Frame *src = (Frame *)get_task_stack_pointer(src_id);

    const char *msg = (const char *)src->r2;
    int msglen = (int)src->r3;

    char *buf = (char *)dest->stk0;
    int buflen = (int)dest->stk1;

    dest->r0 = copy(buf, buflen, msg, msglen);
    src->r0 = dest->r0;

    // if the the Send caller and Reply caller have the same priority,
    // Send caller runs first => dest_id is queued first
    set_task_state(dest_id, TASK_READY);
    push_task(dest_id);
    set_task_state(src_id, TASK_READY);
    push_task(src_id);
}


void kcleanup(void) {
    disable_timer(TIMER_TC1);
    clear_timer(TIMER_TC1);
}


void kinit(void) {
    // Initialize COM2
    bwsetspeed(COM2, 115200);
    bwsetfifo(COM2, OFF);

    print("\r\n");

    init_task_list();
    init_pqueue();
    init_message_queue();
    
    uint *p = (uint *) IVT_BASE_ADDR;
    for (int i = 0; i < 8; i++) {
        *p = (uint)unhandled_exception_handler;
        p = p + 1;
    }
    *(IVT_SWI_ADDR) = (uint)enter_kernel;
    *(IVT_IRQ_ADDR) = (uint)enter_kernel;

    enable_cache();

    *((volatile uint *)0x800B0010) = 0x10;
    //*((volatile uint *)0x800C0010) = 0x8;

    set_timer_mode(TIMER_TC1, 1);
    set_timer_load_value(TIMER_TC1, 20);
    enable_timer(TIMER_TC1);

    DEBUG("kint() finished");
}


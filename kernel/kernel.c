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
#include "uart.h"

#include "interrupt.h"
#include "await.h"


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
    print("\n\r");
    print("Did you ever hear the tragedy of Darth Plagueis The Wise?\n\r");
    print("I thought not. It’s not a story the Jedi would tell you.\n\r");
    print("It’s a Sith legend. Darth Plagueis was a Dark Lord of the\n\r");
    print("Sith, so powerful and so wise he could use the Force to\n\r");
    print("influence the midichlorians to create life. He had such a\n\r");
    print("knowledge of the dark side that he could even keep the\n\r");
    print("ones he cared about from dying. The dark side of the\n\r");
    print("Force is a pathway to many abilities some consider to be\n\r");
    print("unnatural. He became so powerful the only thing he was\n\r");
    print("afraid of was losing his power, which eventually, of\n\r");
    print("course, he did. Unfortunately, he taught his apprentice\n\r");
    print("everything he knew, then his apprentice killed him in his\n\r");
    print("sleep.\n\r\n\r");

    print("Ironic. He could save others from death, but not himself.\n\r");
    print("\n\rPS: This is a panic.\n\r");

    // cleanup and return to redboot
    kcleanup();
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
    clear_vic();
    disable_interrupt(INTERRUPT_TC3UI);
    disable_timer(TIMER_TC1);
    clear_timer(TIMER_TC1);
}


void kinit(void) {
    // start with a clean kernel
    kcleanup();

    // Initialize COM2
    bwsetspeed(COM2, 115200);
    bwsetfifo(COM2, OFF);

    bwsetspeed(COM1, 2400);
    bwsetfifo(COM1, OFF);
    bwtraininitialize(COM1);

    print("\033[2J\033[2r");
    print("\033[s\033[HIDLE: 0%%\t\033[u");
    print("\n\r");

    // the ep93xx reference said to write these values here to enable
    // putting the processor in HALT
    *(SYS_SW_LOCK_ADDR) = 0xAA;
    *(SW_HALT_ENABLE_ADDR) |= 1;

    init_task_list();
    init_pqueue();
    init_message_queue();
    init_event_wait_tid_list();
    
    uint *p = (uint *) IVT_BASE_ADDR;
    for (int i = 0; i < 8; i++) {
        *p = (uint)unhandled_exception_handler;
        p = p + 1;
    }
    *(IVT_SWI_ADDR) = (uint)software_enter_kernel;
    *(IVT_IRQ_ADDR) = (uint)hardware_enter_kernel;

    enable_cache();

    enable_interrupt(INTERRUPT_TC1UI);

    set_timer_mode(TIMER_TC1, 1);
    set_timer_load_value(TIMER_TC1, 20);
    enable_timer(TIMER_TC1);

    enable_interrupt(INTERRUPT_UART1RXINTR1);
    enable_interrupt(INTERRUPT_UART1TXINTR1);
    enable_interrupt(INTERRUPT_UART1);

    DEBUG("kint() finished");
}


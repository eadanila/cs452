#include "user.h"
#include "syscall.h"
#include "logging.h"

#include "timer.h"

#include "name_server.h"

int recv_tid; int tid;
int send_size;

void test_recv4(void) {
    char recv_buf[4];
    char reply_buf[4];
    for (;;) {
        Receive(&tid, recv_buf, 4);
        Reply(tid, reply_buf, 4);
    }
}

void test_recv64(void) {
    char recv_buf[64];
    char reply_buf[64];
    for (;;) {
        Receive(&tid, recv_buf, 64);
        Reply(tid, reply_buf, 64);
    }
}

void test_recv256(void) {
    char recv_buf[256];
    char reply_buf[256];
    for (;;) {
        Receive(&tid, recv_buf, 256);
        Reply(tid, reply_buf, 256);
    }
}

void umain(void) {
    char send_buf[256];
    char reply_buf[256];
    int ticks = 0;
    int recv_pri = 5;

    print("Sender priority\r\n");

    recv_tid = Create(recv_pri, test_recv4);
    ticks = 0;
    stop_debug_timer();
    start_debug_timer();
    for (int j = 0; j < 10000; j++) {
        Send(recv_tid, send_buf, 4, reply_buf, 4);
    }
    ticks = read_debug_timer();
    print("%d tests (size %d), %d ticks\r\n", 10000, 4, ticks);


    recv_tid = Create(recv_pri, test_recv64);
    ticks = 0;
    stop_debug_timer();
    start_debug_timer();
    for (int j = 0; j < 10000; j++) {
        Send(recv_tid, send_buf, 64, reply_buf, 64);
    }
    ticks = read_debug_timer();
    print("%d tests (size %d), %d ticks\r\n", 10000, 64, ticks);


    recv_tid = Create(recv_pri, test_recv256);
    ticks = 0;
    stop_debug_timer();
    start_debug_timer();
    for (int j = 0; j < 10000; j++) {
        Send(recv_tid, send_buf, 256, reply_buf, 256);
    }
    ticks = read_debug_timer();
    print("%d tests (size %d), %d ticks\r\n", 10000, 256, ticks);


    recv_pri = 0;
    print("Receiver priority\r\n")

    recv_tid = Create(recv_pri, test_recv4);
    ticks = 0;
    stop_debug_timer();
    start_debug_timer();
    for (int j = 0; j < 10000; j++) {
        Send(recv_tid, send_buf, 4, reply_buf, 4);
    }
    ticks = read_debug_timer();
    print("%d tests (size %d), %d ticks\r\n", 10000, 4, ticks);


    recv_tid = Create(recv_pri, test_recv64);
    ticks = 0;
    stop_debug_timer();
    start_debug_timer();
    for (int j = 0; j < 10000; j++) {
        Send(recv_tid, send_buf, 64, reply_buf, 64);
    }
    ticks = read_debug_timer();
    print("%d tests (size %d), %d ticks\r\n", 10000, 64, ticks);


    recv_tid = Create(recv_pri, test_recv256);
    ticks = 0;
    stop_debug_timer();
    start_debug_timer();
    for (int j = 0; j < 10000; j++) {
        Send(recv_tid, send_buf, 256, reply_buf, 256);
    }
    ticks = read_debug_timer();
    print("%d tests (size %d), %d ticks\r\n", 10000, 256, ticks);
}




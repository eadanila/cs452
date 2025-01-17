#include "user.h"
#include "syscall.h"
#include "logging.h"

#include "timer.h"

#include "name_server.h"
#include "rps_server.h"
#include "rps_client.h"
#include "clock_server.h"
#include "await.h"
#include "uart.h"
#include "uart_server.h"
#include "print.h"
#include "terminal.h"
#include "tc_server.h"
#include "train_control_server.h"

#define EXPLANATION_COLOR GREEN_TEXT

static int recv_tid; 
static int tid;

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

void send_stuff(void) {
    char send_buf[256];
    char reply_buf[256];
    print("ret code %d\n\r", Send(recv_tid, send_buf, 4, reply_buf, 4));
}

void test_exit(void) {
    Exit();
}

void time_attack(void) {
    char send_buf[256];
    char reply_buf[256];
    int ticks = 0;
    int recv_pri = 5;
    int num = 10000;

    print("Sender priority\n\r");

    recv_tid = Create(recv_pri, test_recv4);
    ticks = 0;
    stop_debug_timer();
    start_debug_timer();
    for (int j = 0; j < num; j++) {
        Send(recv_tid, send_buf, 4, reply_buf, 4);
    }
    ticks = read_debug_timer();
    print("%d tests (size %d), %d ticks\n\r", 10000, 4, ticks);


    recv_tid = Create(recv_pri, test_recv64);
    ticks = 0;
    stop_debug_timer();
    start_debug_timer();
    for (int j = 0; j < num; j++) {
        Send(recv_tid, send_buf, 64, reply_buf, 64);
    }
    ticks = read_debug_timer();
    print("%d tests (size %d), %d ticks\n\r", 10000, 64, ticks);


    recv_tid = Create(recv_pri, test_recv256);
    ticks = 0;
    stop_debug_timer();
    start_debug_timer();
    for (int j = 0; j < num; j++) {
        Send(recv_tid, send_buf, 256, reply_buf, 256);
    }
    ticks = read_debug_timer();
    print("%d tests (size %d), %d ticks\n\r", 10000, 256, ticks);


    recv_pri = 0;
    print("Receiver priority\n\r")

    recv_tid = Create(recv_pri, test_recv4);
    ticks = 0;
    stop_debug_timer();
    start_debug_timer();
    for (int j = 0; j < num; j++) {
        Send(recv_tid, send_buf, 4, reply_buf, 4);
    }
    ticks = read_debug_timer();
    print("%d tests (size %d), %d ticks\n\r", 10000, 4, ticks);

    recv_tid = Create(recv_pri, test_recv64);
    ticks = 0;
    stop_debug_timer();
    start_debug_timer();
    for (int j = 0; j < num; j++) {
        Send(recv_tid, send_buf, 64, reply_buf, 64);
    }
    ticks = read_debug_timer();
    print("%d tests (size %d), %d ticks\n\r", 10000, 64, ticks);


    recv_tid = Create(recv_pri, test_recv256);
    ticks = 0;
    stop_debug_timer();
    start_debug_timer();
    for (int j = 0; j < num; j++) {
        Send(recv_tid, send_buf, 256, reply_buf, 256);
    }
    ticks = read_debug_timer();
    print("%d tests (size %d), %d ticks\n\r", 10000, 256, ticks);
}

void rps_tests()
{
    Create(0, rps_server);

    // NOTE: a client cannot quit while on queue since 
    //       they are blocked until an opponent is found

    print(EXPLANATION_COLOR);
    print("Both clients play once then quit:\n\r");
    print(RESET_FORMATTING);

    Create(1, rps_rock_client);
    Create(1, rps_paper_client);



    bwgetc(COM2);
    print(EXPLANATION_COLOR);
    print("\n\rBoth clients play 3 times before quitting:\n\r");
    print(RESET_FORMATTING);

    Create(1, rps_rock_lover_client);
    Create(1, rps_paper_lover_client);



    bwgetc(COM2);
    print(EXPLANATION_COLOR);
    print("\n\rBoth clients don't play and quit immediately:\n\r");
    print(RESET_FORMATTING);

    Create(1, rps_quitter_client);
    Create(1, rps_quitter_client);



    bwgetc(COM2);
    print(EXPLANATION_COLOR);
    print("\n\rSecond client doesnt play and quits immediately:\n\r");
    print(RESET_FORMATTING);

    Create(1, rps_rock_lover_client);
    Create(1, rps_quitter_client);



    bwgetc(COM2);
    print(EXPLANATION_COLOR);
    print("\n\rFirst client doesn't play and quits immediately:\n\r");
    print(RESET_FORMATTING);

    Create(1, rps_quitter_client);
    Create(1, rps_rock_lover_client);



    bwgetc(COM2);
    print(EXPLANATION_COLOR);
    print("\n\rAn abusive client and a normal client that plays once are created.");
    print("\n\rWithout initially signing up, the abusive client tries to");
    print("\n\rQuit -> Play(PAPER) -> Signup -> Quit -> Play(PAPER):\n\r");
    print(RESET_FORMATTING);

    Create(1, rps_abusive_client);
    Create(1, rps_rock_client);



    bwgetc(COM2);
    print(EXPLANATION_COLOR);
    print("\n\rFirst client higher priority than second:\n\r");
    print(RESET_FORMATTING);
    
    Create(1, rps_rock_client);
    Create(2, rps_paper_client);



    bwgetc(COM2);
    print(EXPLANATION_COLOR);
    print("\n\rFirst client lower priority than second:\n\r");
    print(RESET_FORMATTING);

    Create(2, rps_rock_client);
    Create(1, rps_paper_client);



    bwgetc(COM2);
    print(EXPLANATION_COLOR);
    print("\n\rTwo clients try every combination of games possible:\n\r");
    print(RESET_FORMATTING);

    Create(2, rps_try_everything_1);
    Create(2, rps_try_everything_2);



    bwgetc(COM2);
    print(EXPLANATION_COLOR);
    print("\n\rFour clients join and begin playing.");
    print("\n\rFirst two play 4 times and the last two play 3 times.\n\r");
    print(RESET_FORMATTING);

    Create(3, rps_player_1);
    Create(3, rps_player_2);
    Create(3, rps_player_3);
    Create(3, rps_player_4);
}

void name_test_1()
{
    RegisterAs("AAA");
    print("test_1 is %d \n\r", MyTid());
}

void name_test_2()
{
    RegisterAs("BBB");
    print("test_2 is %d \n\r", MyTid());
}

void name_test_3()
{
    RegisterAs("BBB");
    print("test_3 is %d \n\r", MyTid());
}

void name_server_test()
{
    Create(0, name_test_1);
    Create(0, name_test_2);

    print("who is AAA?: %d\n\r", WhoIs("AAA"));
    print("who is BBB?: %d\n\r", WhoIs("BBB"));
    print("who is CCC?: %d\n\r", WhoIs("CCC"));

    Create(0, name_test_3);

    print("who is AAA?: %d\n\r", WhoIs("AAA"));
    print("who is BBB?: %d\n\r", WhoIs("BBB"));
    print("who is CCC?: %d\n\r", WhoIs("CCC"));
}

void clock_client()
{
    char message[1];
    char reply[2];

    Send(MyParentTid(), message, 0, reply, 2);
    // Delay interval is now in reply[0] and number of delays is in reply[1]

    int cs_id = WhoIs("clock_server");
    int my_tid = MyTid();

    for(int i = 0; i != reply[1]; i++)
    {
        Delay(cs_id, reply[0]);
        print("Tid: %d, Delay Interval: %d, Delays completed: %d\n\r", my_tid, reply[0], i+1);
        // print("Tid: %d, Delay Interval: %d, Delays completed: %d, Total delay: %d\n\r", my_tid, reply[0], i+1, (i+1)*reply[0]);
    } 
}

void clock_server_test()
{
    // Create clock and name servers
    name_server_id = Create(0, name_server);
    Create(0, clock_server);

    // Create 4 clients with required priorties
    int c1 = Create(3, clock_client);
    int c2 = Create(4, clock_client);
    int c3 = Create(5, clock_client);
    int c4 = Create(6, clock_client);

    char msg[2];

    int dummy;

    Receive(&dummy, msg, 0);
    Receive(&dummy, msg, 0);
    Receive(&dummy, msg, 0);
    Receive(&dummy, msg, 0);

    // c1++;
    // c2++;
    // c3++;
    // c4++;

    msg[0] = 10; msg[1] = 20; Reply(c1, msg, 2);
    msg[0] = 23; msg[1] = 9; Reply(c2, msg, 2);
    msg[0] = 33; msg[1] = 6; Reply(c3, msg, 2);
    msg[0] = 71; msg[1] = 3; Reply(c4, msg, 2);
}

// 0   1   2   3
// LSB         MSB
// ┬
// └─── pointer
// When index 0 is cast to an int pointer, ints are placed in memory as shown above.
// The int populates memory in increasing order.

void byte_alignment_test()
{
    char test[32];
    for(int i = 0; i != 32; i++) test[i] = 0;

    *((int*)&test[0]) = 0xdeadbeef;

    print("\n\r");

    for(int i = 0; i != 32; i++) {print("%x", test[i]); print("\n\r");}
}

void clock_server_error_test()
{
    int cs = WhoIs("clock_server");

    print("Should get \n\r-1, -1, TIME, -1, -1, -2, TIME, -1, -1, -2, TIME \n\r");

    print("Should be: -1,   Got %d\n\r", Time(21));
    print("Should be: -1,   Got %d\n\r", Time(-20));
    Delay(cs, 5);
    print("Should be: TIME, Got %d\n\r", Time(cs));
    print("Should be: -1,   Got %d\n\r", Delay(21, 10));
    print("Should be: -1,   Got %d\n\r", Delay(-21, 10));
    print("Should be: -2,   Got %d\n\r", Delay(cs, -20));
    print("Should be: TIME, Got %d\n\r", Delay(cs, 23));
    print("Should be: -1,   Got %d\n\r", DelayUntil(21, 23));
    print("Should be: -1,   Got %d\n\r", DelayUntil(-21, 10));
    print("Should be: -2,   Got %d\n\r", DelayUntil(cs, -20));
    print("Should be: TIME, Got %d\n\r", DelayUntil(cs, 23));
}

void empty_spinner()
{
    int s = 0;
    s++;
    for(;;){}
}

void umain(void)
{
    name_server_id = Create(0, name_server);
    Create(0, clock_server);
    create_uart_servers();
    Create(3, tc_server);
    // Create(3, empty_spinner);
    Create(4, train_control_server);
    Create(5, terminal);

    // uart_send_byte(UART1, 16);
    // AwaitEvent(EVENT_UART1_CTS_LOW);
    // AwaitEvent(EVENT_UART1_CTS_HIGH);
    // AwaitEvent(EVENT_UART1_TX_INTERRUPT);
    // uart_send_byte(UART1, 24);
    // AwaitEvent(EVENT_UART1_CTS_LOW);
    // AwaitEvent(EVENT_UART1_CTS_HIGH);
    // AwaitEvent(EVENT_UART1_TX_INTERRUPT);
    
    // TODO Perhaps move to kernel and #define the id
    // name_server_id = Create(0, name_server);
    // Create(0, clock_server);

    // Create(3, clock_server_test);

    // Create(3, clock_server_error_test);

    // Create(2, byte_alignment_test);

    // Create(4, time_attack);

    // Create(3, rps_tests);

    // Create(0, name_server_test);
    
    ////////////////////////////////////////

    // Create(0, name_test);

    ////////////////////////////////////////

    // int id = 0;
    // // 3 is lower priority than 1
    // id = Create(3, recv_task);
    // print("Created: %d\n\r", id)

    // id = Create(3, send_task);
    // print("Created: %d\n\r", id);

    // // Send(0xbadf00d, (char *)0xdeadbeef, 0x12345, (char *)0x67890, 0xabcdef);
    
    // for (int i = 0; i < 1000; i++) {
    //     AwaitEvent(EVENT_TIMER1_INTERRUPT);
    // }

    // print("umain: exiting\n\r");
    // print("FirstUserTask: exiting\n\r");
}

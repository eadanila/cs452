#include "user.h"
#include "syscall.h"
#include "logging.h"

#include "timer.h"

#include "name_server.h"

void recv_task(void)
{
    char buf[100];
    int bufsz = 100;
    int tid;

    int len = Receive(&tid, buf, bufsz);

    print("Got message of size %d from %d\r\n", len, tid);
    print("Message was: %s\r\n", buf);

    Reply(2, "thanks!", 8);
}

void send_task(void)
{
    char buf[128];
    int bufsz = 128;
    char rpbuf[64];
    int rpbuflen = 64;

    for (int i = 0; i < bufsz; i++) {
        buf[i] = 'a';
        if (i == 55)
            buf[i] = 'b';
    }

    int high, low;
    start_debug_timer();
    stop_debug_timer();
    start_debug_timer();
    int len = Send(1, buf, bufsz, rpbuf, rpbuflen);
    read_debug_timer(&high, &low);
    print("TIME: %d ticks\r\n", low);

    print("Got reply of size %d from somebody\r\n", len);
    print("Message was: %s\r\n", rpbuf);
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

void name_test()
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

void umain(void)
{
    // TODO Perhaps move to kernel and #define the id
    name_server_id = Create(0, name_server);

    Create(0, name_test);

//     int id = 0;
//     // 3 is lower priority than 1
//     id = Create(3, recv_task);
//     print("Created: %d\r\n", id)

//     id = Create(3, send_task);
//     print("Created: %d\r\n", id);

// //    Send(0xbadf00d, (char *)0xdeadbeef, 0x12345, (char *)0x67890, 0xabcdef);

//     print("FirstUserTask: exiting\r\n");

//     Exit();
}

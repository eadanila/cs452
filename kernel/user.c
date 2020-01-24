#include "user.h"
#include "syscall.h"
#include "logging.h"

void test_task(void)
{
    print("Task ID: %d, Parent ID: %d\r\n", MyTid(), MyParentTid());
    Yield();
    print("Task ID: %d, Parent ID: %d\r\n", MyTid(), MyParentTid());
    Exit();
}

void umain(void)
{
    // 3 is lower priority than 1
    DEBUG("");
    Create(3, test_task);
    Create(3, test_task);

    Create(1, test_task);
    Create(1, test_task);

    print("FirstUserTask: exiting\r\n");

    Exit();
}
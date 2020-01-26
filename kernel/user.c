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
    int id = 0;
    // 3 is lower priority than 1
    id = Create(3, test_task);
    print("Created: %d\r\n", id)

    id = Create(3, test_task);
    print("Created: %d\r\n", id);

    id = Create(1, test_task);
    print("Created: %d\r\n", id);

    id = Create(1, test_task);
    print("Created: %d\r\n", id);

    print("FirstUserTask: exiting\r\n");

    Exit();
}

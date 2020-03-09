#include "idle_printer.h"

#include "terminal.h"
#include "name_server.h"
#include "clock_server.h"

void idle_printer(void)
{
    int tid = WhoIs("terminal_output");
    int cid = WhoIs("clock_server");

    for (int i = 0;; i++) {
        Delay(cid, 10);
        TPrint(tid, "\033[s\033[HIDLE: %d%%\t\033[u", idle_time/(end_time/100));
    }
}
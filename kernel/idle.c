#include "logging.h"
#include "idle.h"
#include "kernel.h"

void idle_task(void) {
    for (int i = 0; ; i++)
        print("\033[s\033[H\033[2KIDLE: %d%%\n\r\033[u", idle_time/(end_time/100))
}


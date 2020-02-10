#include "logging.h"
#include "constants.h"
#include "idle.h"

void idle_task(void) {
    for (int i = 0; ; i++) {
        print("\033[s\033[HIDLE: %d%%\t\033[u", idle_time/(end_time/100));
        *(HALT_MODE_ADDR);
    }
}


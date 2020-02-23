#include "logging.h"
#include "constants.h"
#include "idle.h"

void idle_task(void) {
    for (;;) *(HALT_MODE_ADDR);
}


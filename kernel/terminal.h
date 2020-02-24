#ifndef TERMINAL_H
#define TERMINAL_H

#include "constants.h"

// The terminal should be used to collect all prints. 
// TPrint prints a formatted string where the terminal task sees fit.
void TPrint(int tid, char* str, ...);

void terminal(void);

#endif
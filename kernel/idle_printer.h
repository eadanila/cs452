#ifndef IDLE_PRINTER_H
#define IDLE_PRINTER_H

#include "constants.h"

#define IDLE_HISTORY 10

typedef struct idle_buffer IdleBuffer;

struct idle_buffer
{
	uint data[IDLE_HISTORY];
	int size;
	int start;
	int end;

    uint idle_time; // In debug timer ticks
};

uint idle_time, start_time, end_time;
IdleBuffer idle_buffer;

void init_idle_buffer(IdleBuffer* r);
void add_measurement(IdleBuffer *b, uint idle_time);

void idle_printer(void);

#endif
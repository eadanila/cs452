#include "idle_printer.h"

#include "terminal.h"
#include "name_server.h"
#include "clock_server.h"

void init_idle_buffer(IdleBuffer* r)
{
	r->size = 0;
	r->start = 0;
	r->end = 0;
    r->idle_time = 0;

    for(int i = 0; i != IDLE_HISTORY; i++) r->data[i] = 0;
}

void add_measurement(IdleBuffer *b, uint idle_time)
{
    if(b->size != IDLE_HISTORY) b->size++;
    else 
    {
        // An old time is being overwritten and thus
        // removed from the buffer
        b->idle_time -= b->data[b->end];
    }

	b->data[b->end] = idle_time;
    b->idle_time += idle_time;
	
	b->end = (b->end + 1)%IDLE_HISTORY;
}

void idle_printer(void)
{
    int tid = WhoIs("terminal_output");
    int cid = WhoIs("clock_server");

    for (;;) {
        Delay(cid, 10);
        int total_time = (983*idle_buffer.size/10); // In 10 ms ticks
        TPrint(tid, "\033[s\033[HIDLE: %d%%  \t\033[u", idle_buffer.idle_time/total_time);

        if(idle_buffer.size == IDLE_HISTORY) 
        {
            uint min_idle = idle_buffer.data[0];
            for(int i = 0; i != IDLE_HISTORY; i++)
            {
                if(idle_buffer.data[i] < min_idle) min_idle = idle_buffer.data[i];
            } 

            // One tick is 9830 debug clock ticks
            TPrintAt(tid, 14, 0, "MIN TICK IDLE: %d%%  \t\033[u", 100*min_idle/9830);
        }
    }
}
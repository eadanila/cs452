#ifndef DELAYED_EVENT_H
#define DELAYED_EVENT_H

// event will send a message with this header after the delay has elapsed
#define EVENT_OCCURED 16

void event();
int create_event(int delay);

#endif
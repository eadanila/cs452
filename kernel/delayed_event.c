#include "delayed_event.h"
#include "name_server.h"
#include "clock_server.h"
#include "syscall.h"
#include "string_utility.h"
#include "logging.h"

void event()
{
    int cid = WhoIs("clock_server");
    int sender;
    char msg[4];

    // Receive integer delay and respond instantly
    Receive(&sender, msg, 4);
    Reply(sender, msg, 0);

    // Delay for time received
    int time = Delay(cid, unpack_int(msg));

    // After the delay, send back the current time!
    char response[5];
    response[0] = EVENT_OCCURED;
    pack_int(time, response + 1);
    char reply;
    Send(sender, response, 5, &reply, 0);
}

int create_event(int delay)
{
    assert(delay > 0);
    // Create event task
    int event_id = Create(3, event);
    char msg[4];
    char reply[1];

    // Send the delay duration
    pack_int(delay, msg);
    Send(event_id, msg, 5, reply, 0); // Will get replied to instantly 

    // return the tid of the task as the event id
    return event_id;
}
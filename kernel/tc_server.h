#ifndef TC_SERVER_H
#define TC_SERVER_H

#include "track_constants.h"

#define INVALID_TC_SERVER -1
#define INVALID_ARGUMENT -2

// Atomic command to be sent to the UART1 server associated with 
// a delay to be waited until another command is executed on its queue.
// There are seperate queues (in CommandQueue) for each train and one 
// for switch commands.
// Can hold up to 2 characters as thats the maximum length of any
// command we should be sending. If only 1 byte is needed for the
// command (such as 32 to turn off solenoid), place a NO_COMMAND
// into the second byte. 
// A command can store the tid of a task to unblock when it eventually
// reaches the front of its respective queue, and gets sent to the
// UART1 server.
typedef struct command Command;
// A circular buffer storing Commands
typedef struct command_ring_buffer CommandRingBuffer;
// Structure holding a seperate CommandRingBuffer for each train, and 
// one for switch commands.
typedef struct command_queue CommandQueue;

struct command
{
    int blocked_task;

    char arg0;
    char arg1;
    int delay;
};

// NOTE: This is an initial interface meant to satisfy the requirements 
//       for a0 and will expand/change during TC1 and TC2.

// Blocks until command is sent to the uart server
int SwitchTrack(int tid, char id, char dir);

// Return immediatly
int SwitchTrackAsync(int tid, char id, char dir);

// Returns immediatly
int SetSpeed(int tid, char id, char speed);

// Returns immediatly
int Reverse(int tid, char id);

// Blocks until sensor dump is requested and fully received
int GetSensors(int tid, char* buffer);

// Blocks until the tc server is finished initializing the track
int InitComplete(int tid);

void tc_server(void);

#endif

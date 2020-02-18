#include "tc_server.h"
#include "syscall.h"
#include "uart_server.h"

#define MAX_REQUEST_LENGTH 3

#define SWITCH_TRACK_COMMAND 1
#define SET_SPEED_COMMAND 2
#define REVERSE_COMMAND 3

int SwitchTrack(int tid, char id, char dir)
{
    char message[3];
    char reply[1];

    message[0] = SWITCH_TRACK_COMMAND;
    message[1] = id;
    message[2] = dir;

    // If tid is not the task id of an existing task.
    if(Send(tid, message, 3, reply, 0) == -1) 
        return INVALID_TC_SERVER;

    return 0;
}

int SetSpeed(int tid, char id, char speed)
{
    char message[3];
    char reply[1];

    message[0] = SET_SPEED_COMMAND;
    message[1] = id;
    message[2] = speed;

    // If tid is not the task id of an existing task.
    if(Send(tid, message, 3, reply, 0) == -1) 
        return INVALID_TC_SERVER;

    return 0;
}

int Reverse(int tid, char id)
{
    char message[2];
    char reply[1];

    message[0] = REVERSE_COMMAND;
    message[1] = id;

    // If tid is not the task id of an existing task.
    if(Send(tid, message, 2, reply, 0) == -1) 
        return INVALID_TC_SERVER;

    return 0;
}

void tc_server(void)
{

}
#include "tc_server.h"
#include "syscall.h"
#include "uart_server.h"
#include "string_utility.h"
#include "name_server.h"
#include "clock_server.h"
#include "logging.h"

#define MAX_REQUEST_LENGTH 11

#define SWITCH_TRACK_COMMAND 1
#define SET_SPEED_COMMAND 2
#define REVERSE_COMMAND 3
#define GET_SENSORS_COMMAND 7

#define TIME_CHANGED 4
#define SENSOR_DUMP 5
#define NOTIFIER_STARTED 6

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

int GetSensors(int tid, char* buffer)
{
    char message[1];

    message[0] = GET_SENSORS_COMMAND;

    // If tid is not the task id of an existing task.
    if(Send(tid, message, 1, buffer, 10) == -1) 
        return INVALID_TC_SERVER;

    return 0;
}

// This is needed to check command delay queues for what should be exectuted.
void tick_notifier()
{
    int tcid = WhoIs("tc_server");
    int csid = WhoIs("clock_server");

    char message[5];
    char reply[1];

    message[0] = TIME_CHANGED;

    for(;;)
    {
        int time = Delay(csid, 1);
        pack_int(time, message + 1);

        // Notify server that time has changed
        Send(tcid, message, 5, reply, 0);
    }
}

void sensor_dump_notifier()
{
    int tcid = WhoIs("tc_server");
    int uid = WhoIs("com1");

    char message[11];
    char reply[1];

    message[0] = NOTIFIER_STARTED;

    for(;;)
    {
        // Wait to get request to read sensor data
        Send(tcid, message, 11, reply, 0);

        message[0] = SENSOR_DUMP;
        Putc(uid, COM1, 128 + 5);

        for(int i = 0; i != 10; i++)
        {
            message[1+i] = Getc(uid, COM1);
        }
    }
}

void tc_server(void)
{
    RegisterAs("tc_server");

    int uid = WhoIs("com1");

    int sender;
    char msg[MAX_REQUEST_LENGTH];
    char reply_msg[1];

    int time = -1;
    int waiting_for_sensor_dump = -1; // TODO Add support for multiple waiting for sensor commands.

    Create(1, tick_notifier);
    int sensor_dump_notifier_id = Create(1, sensor_dump_notifier);

    for(;;)
    {
        // Receive clock server request
        Receive(&sender, msg, MAX_REQUEST_LENGTH);

        // Execute the command encoded in the request
        switch(msg[0])
        {
            case TIME_CHANGED:
                time = unpack_int(msg + 1);
                time++; time--; // TEMP To suppress error

                Reply(sender, reply_msg, 0);
                break;

            case SENSOR_DUMP:
                assert(waiting_for_sensor_dump != -1);

                // Sensor dump notifier finished obtaining the requested sensor dump.
                // Reply to the waiting task with the data.
                // TODO Have sensor_dump_notifer respond to the task directly instead.
                Reply(waiting_for_sensor_dump, msg + 1, 10);
                break;

            case NOTIFIER_STARTED:
                // Do nothing
                break;

            case SWITCH_TRACK_COMMAND:
                
                break;

            case SET_SPEED_COMMAND:
                Putc(uid, COM1, msg[2]); // speed
                Putc(uid, COM1, msg[1]); // id
                Reply(sender, msg, 0);
                break;

            case REVERSE_COMMAND:
                
                break;

            case GET_SENSORS_COMMAND:
                waiting_for_sensor_dump = sender;

                // Wake up sensor_dump_notifier. It will send a sensor dump when finished.
                Reply(sensor_dump_notifier_id, reply_msg, 0);
                break;
        }
    }
}
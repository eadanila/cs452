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

#define TRAIN_COUNT 6
#define SWITCH_COUNT 22
#define MAX_TRAIN_NUMBER 79

#define COMMAND_BUFFER_SIZE 256

// NO_COMMAND should be a value that would never be used in a command
#define NO_COMMAND 255
#define TURN_OFF_SOLENOID 32
#define SWITCH_DELAY 20
#define TURN_OFF_SOLENOID_DELAY 15

typedef struct command Command;
typedef struct command_ring_buffer CommandRingBuffer;
typedef struct command_queue CommandQueue;

struct command
{
    int blocked_task;

    char arg0;
    char arg1;
    int delay;
};

Command create_command(char arg0, char arg1, int delay)
{
    Command result;
    result.blocked_task = -1;
    result.arg0 = arg0;
    result.arg1 = arg1;
    result.delay = delay;

    return result;
}

Command create_unblocking_command(int blocked_task, char arg0, char arg1, int delay)
{
    Command result;
    result.blocked_task = blocked_task;
    result.arg0 = arg0;
    result.arg1 = arg1;
    result.delay = delay;

    return result;
}

struct command_ring_buffer
{
	Command data[COMMAND_BUFFER_SIZE];
	int size;
	int start;
	int end;

    char executed;
};

void init_command_buffer(CommandRingBuffer* r)
{
	r->size = 0;
	r->start = 0;
	r->end = 0;

    r->executed = 0;
}

void add_command(CommandRingBuffer *b, Command c)
{
	b->data[b->end] = c;
	if(b->size != COMMAND_BUFFER_SIZE) b->size++;

	b->end = (b->end + 1)%COMMAND_BUFFER_SIZE;
}

// Always check size first
Command remove_command(CommandRingBuffer *b)
{
	Command r = b->data[b->start]; 
	b->start = (b->start + 1)%COMMAND_BUFFER_SIZE;
	b->size--;
	return r;
}

Command* peek_command(CommandRingBuffer *b)
{
    if(b->size == 0) return 0;
	return &(b->data[b->start]);
}

struct command_queue
{
    CommandRingBuffer train_commands[TRAIN_COUNT];
    CommandRingBuffer switch_commands;
};

void init_command_queue(CommandQueue* cq)
{
    for(int i = 0; i != TRAIN_COUNT; i++)
    {
       init_command_buffer(&cq->train_commands[i]);
    }
    init_command_buffer(&cq->switch_commands);
}

// Exectutes command first, then waits for a delay until the next command is sent.
void process_command_queue(int tid, CommandRingBuffer* crb, int dt)
{
    Command* current_command = peek_command(crb);

    while(current_command && dt >= 0)
    {
        if(!crb->executed) 
        {
            // Send appropriate put based on number of bytes in command
            if(current_command->arg1 == NO_COMMAND)
                Putc(tid, COM1, current_command->arg0);
            else
                PutCommand(tid, COM1, current_command->arg0, current_command->arg1);
                // Putc(tid, COM1, current_command->data[0]);
                // Putc(tid, COM1, current_command->data[1]);

            crb->executed = 1;
        }

        if(current_command->delay <= dt)
        {
            // Execute the command
            dt -= current_command->delay;
            
            remove_command(crb);
            current_command = peek_command(crb);
            crb->executed = 0;
        }
        else
        {
            current_command->delay -= dt;
            return;
        }
    }
}

void process_commands(int tid, CommandQueue* cq, int dt)
{
    // Process all train commands that are ready
    for(int i = 0; i != TRAIN_COUNT; i++)
    {
       process_command_queue(tid, &cq->train_commands[i], dt);
    }

    // Process all switch commands that are ready
    process_command_queue(tid, &cq->switch_commands, dt);
}

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
    int csid = WhoIs("clock_server");

    int sender;
    char msg[MAX_REQUEST_LENGTH];
    char reply_msg[1];

    Create(1, tick_notifier);
    int sensor_dump_notifier_id = Create(1, sensor_dump_notifier);

    int new_time;
    int time = Time(csid);
    int waiting_for_sensor_dump = -1; // TODO Add support for multiple waiting for sensor commands.

    // Initialize train constants
    int train_ids[] = {1, 24, 58, 74, 78, 79};
    int train_to_enumeration[MAX_TRAIN_NUMBER + 1];
    for(int i = 0; i != MAX_TRAIN_NUMBER + 1; i++) train_to_enumeration[i] = -1;
    for(int i = 0; i != TRAIN_COUNT; i++) train_to_enumeration[train_ids[i]] = i;

    int switch_ids[SWITCH_COUNT];
	for(int i = 0; i != 18; i++) switch_ids[i] = i + 1;
	switch_ids[18] = 0x99;
	switch_ids[19] = 0x9A;
	switch_ids[20] = 0x9B;
	switch_ids[21] = 0x9C;

    // switch_ids[0]++;  switch_ids[0]--;

    int train_speeds[TRAIN_COUNT];
    for(int i = 0; i != TRAIN_COUNT; i++) train_speeds[i] = 0;

    CommandQueue command_queue;
    init_command_queue(&command_queue);

    // Set all train speeds to 0
    for(int i = 0; i != 4; i++) // 1 -> 6,  2 -> 9 , 3 -> 12, 4 -> 15, 5-> 18, 6 -> 21
        add_command(&command_queue.train_commands[i], 
                    create_command(0, msg[1], 200));

    // Set all switches to straight
    for(int i = 0; i != 1; i++) 
    {
        add_command(&command_queue.switch_commands, create_command(STRAIGHT, 
                                                                   switch_ids[i], 
                                                                   SWITCH_DELAY));
    }
    add_command(&command_queue.switch_commands, create_command(TURN_OFF_SOLENOID, 
                                                                NO_COMMAND, 
                                                                TURN_OFF_SOLENOID_DELAY));

    for(;;)
    {
        // Receive clock server request
        Receive(&sender, msg, MAX_REQUEST_LENGTH);

        // Execute the command encoded in the request
        switch(msg[0])
        {
            case TIME_CHANGED:
                new_time = unpack_int(msg + 1);
                process_commands(uid, &command_queue, new_time - time);
                time = new_time;
                time ++; time --;

                Reply(sender, reply_msg, 0);
                break;

            case SENSOR_DUMP:
                assert(waiting_for_sensor_dump != -1);

                // Sensor dump notifier finished obtaining the requested sensor dump.
                // Reply to the waiting task with the data.
                // TODO Have sensor_dump_notifer respond to the task directly instead.
                Reply(waiting_for_sensor_dump, msg + 1, 10);
                waiting_for_sensor_dump = -1;
                break;

            case NOTIFIER_STARTED:
                // Do nothing
                break;

            case SWITCH_TRACK_COMMAND:
                add_command(&command_queue.switch_commands, create_command(msg[2], msg[1], SWITCH_DELAY));

                // TODO In future, only add turn off solenoid at the end!
                add_command(&command_queue.switch_commands, create_command(TURN_OFF_SOLENOID, 
                                                                           NO_COMMAND, 
                                                                           TURN_OFF_SOLENOID_DELAY));
                // TODO Make block
                Reply(sender, reply_msg, 0);
                break;

            case SET_SPEED_COMMAND:
                assert(train_to_enumeration[(int)msg[1]] != -1);

                add_command(&command_queue.train_commands[train_to_enumeration[(int)msg[1]]], 
                        create_command(msg[2], msg[1], 0));
                train_speeds[train_to_enumeration[(int)msg[1]]] = msg[2];

                Reply(sender, msg, 0);
                // Putc(uid, COM1, msg[2]); // speed
                // Putc(uid, COM1, msg[1]); // id
                // Reply(sender, msg, 0);
                break;

            case REVERSE_COMMAND:
                // TODO Make a reverse command when train is stopped take 0 time?

                // TODO Create more accurate function for stopping time
                // Stop train
                add_command(&command_queue.train_commands[train_to_enumeration[(int)msg[1]]], 
                            create_command(0, msg[1], (train_speeds[train_to_enumeration[(int)msg[1]]]/4 + 1)*100 ));

                // Send reverse command
                add_command(&command_queue.train_commands[train_to_enumeration[(int)msg[1]]], 
                            create_command(15, msg[1], 0));

                // Set speed to old speed
                // TODO Add a current train speed tracker that gets changed when command is sent
                add_command(&command_queue.train_commands[train_to_enumeration[(int)msg[1]]], 
                            create_command(train_speeds[train_to_enumeration[(int)msg[1]]], msg[1], 0));

                Reply(sender, msg, 0);
                break;

            case GET_SENSORS_COMMAND:
                waiting_for_sensor_dump = sender;

                // Wake up sensor_dump_notifier. It will send a sensor dump when finished.
                Reply(sensor_dump_notifier_id, reply_msg, 0);
                break;
        }
    }
}
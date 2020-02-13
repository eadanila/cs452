#include "uart_server.h"
#include "bwio.h"
#include "syscall.h"
#include "logging.h"

#define GET_CHAR 1
#define PUT_CHAR 2

// First byte is the channel, Second is the operation
#define US_REQUEST_LENGTH 2
#define US_REPLY_LENGTH 1


int Getc(int tid, int channel)
{
    assert(channel == COM1 || channel == COM2);

    char message[US_REQUEST_LENGTH];
    char reply[US_REPLY_LENGTH];

    message[0] = channel;

    // if tid is not the task id of an existing task.
    if(Send(tid, message, US_REQUEST_LENGTH, reply, US_REPLY_LENGTH) == -1) 
        return INVALID_UART_SERVER;

    return reply[0];
}

int Putc(int tid, int channel, char ch)
{
    assert(channel == COM1 || channel == COM2);

    char message[US_REQUEST_LENGTH];
    char reply[US_REPLY_LENGTH];

    message[0] = channel;
    message[1] = ch;

    // if tid is not the task id of an existing task.
    if(Send(tid, message, US_REQUEST_LENGTH, reply, US_REPLY_LENGTH) == -1) 
        return INVALID_UART_SERVER;

    return reply[0];
}

void com1_getc_notifer(void)
{

}

void com1_putc_worker(void)
{

}

void com2_getc_notifer(void)
{

}

void com2_getc_worker(void)
{

}

void uart_server(void)
{

}

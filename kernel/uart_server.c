#include "uart_server.h"
#include "bwio.h"
#include "syscall.h"
#include "logging.h"
#include "name_server.h"
#include "uart.h"
#include "await.h"

#define GET_CHAR 1
#define PUT_CHAR 2
#define CHAR_RECEIVED 3
#define PUT_READY 4

// First byte is the channel, Second is the operation
#define US_REQUEST_LENGTH 2
#define US_REPLY_LENGTH 1


#define BUFFER_SIZE 1024

typedef struct ring_buffer RingBuffer;

struct ring_buffer
{
	char data[BUFFER_SIZE];
	int size;
	int start;
	int end;
};

void init_buffer(RingBuffer* r)
{
	r->size = 0;
	r->start = 0;
	r->end = 0;
}

void add_byte(RingBuffer *b, char c)
{
	b->data[b->end] = c;
	if(b->size != BUFFER_SIZE) b->size++;

	b->end = (b->end + 1)%BUFFER_SIZE;
}

// Always check size first
char remove_byte(RingBuffer *b)
{
	char r = b->data[b->start]; 
	b->start = (b->start + 1)%BUFFER_SIZE;
	b->size--;
	return r;
}


int Getc(int tid, int channel)
{
    // Currently channel is ignored
    // assert(channel == COM1 || channel == COM2);

    char message[US_REQUEST_LENGTH];
    char reply[US_REPLY_LENGTH];

    message[0] = GET_CHAR;

    // if tid is not the task id of an existing task.
    if(Send(tid, message, US_REQUEST_LENGTH, reply, US_REPLY_LENGTH) == -1) 
        return INVALID_UART_SERVER;

    return reply[0];
}

int Putc(int tid, int channel, char ch)
{
    // Currently channel is ignored
    // assert(channel == COM1 || channel == COM2);

    char message[US_REQUEST_LENGTH];
    char reply[US_REPLY_LENGTH];

    message[0] = PUT_CHAR;
    message[1] = ch;

    // if tid is not the task id of an existing task.
    if(Send(tid, message, US_REQUEST_LENGTH, reply, US_REPLY_LENGTH) == -1) 
        return INVALID_UART_SERVER;

    return 0;
}

void uart1_getc_notifer(void)
{

}

void uart1_putc_notifer(void)
{
    int s_id = WhoIs("com1");

    for(;;)
    {
        char message[US_REQUEST_LENGTH];
        char reply[US_REPLY_LENGTH];
        message[0] = PUT_READY;

        // Get char to print
        Send(s_id, message, US_REQUEST_LENGTH, reply, US_REPLY_LENGTH);

        AwaitEvent(EVENT_UART1_CTS_LOW);
        AwaitEvent(EVENT_UART1_CTS_HIGH);
        AwaitEvent(EVENT_UART1_TX_INTERRUPT);
    }
}

void uart2_getc_notifer(void)
{

}

void uart2_putc_notifer(void)
{

}

void uart1_server(void)
{
    // Register on the name server
    RegisterAs("com1");

    // Notifiers and must be one priority level higher
    /* int uart1_getc_notifer_tid  = */ Create(0, uart1_getc_notifer);
     int uart1_putc_notifer_tid  = Create(0, uart1_putc_notifer);
    /* int uart2_getc_notifer_tid  = */ Create(0, uart2_getc_notifer);
    /* int uart2_putc_notifer_tid  = */ Create(0, uart2_putc_notifer);

    RingBuffer put_buffer;
    RingBuffer get_buffer;

    int queued_getc_task = -1;
    int put_ready = 1;

    init_buffer(&put_buffer);
    init_buffer(&get_buffer);

    int sender;
    char msg[US_REQUEST_LENGTH];
    char reply_msg[US_REPLY_LENGTH];

    for(;;)
    {
        // Receive clock server request
        Receive(&sender, msg, US_REQUEST_LENGTH);

        // Execute the command encoded in the request
        switch(msg[0])
        {
            case GET_CHAR:
                if(get_buffer.size > 0) 
                {
                    reply_msg[0] = remove_byte(&get_buffer);
                    Reply(sender, reply_msg, US_REPLY_LENGTH);
                }
                else if(queued_getc_task == -1)
                {
                    queued_getc_task = sender;
                }
                else 
                {
                    reply_msg[0] = OTHER_TASK_QUEUED;
                    Reply(sender, reply_msg, US_REPLY_LENGTH);
                }
                break;

            case PUT_CHAR:
                add_byte(&put_buffer, msg[1]);
                Reply(sender, reply_msg, US_REPLY_LENGTH);
                break;

            case CHAR_RECEIVED:
                // Currently do nothing
                break;

            case PUT_READY:
                // TODO Create notifers in the server, and add asserts so 
                // that other tasks cant send these commands
                put_ready = 1;
                break;
        }

        if(put_ready && put_buffer.size > 0)
        {
            Reply(uart1_putc_notifer_tid, reply_msg, 0);
            char c = remove_byte(&put_buffer);
            uart_send_byte(UART1, c);

            put_ready = 0;
        }
    }
}

void uart2_server(void)
{

}

void create_uart_servers(void)
{
    Create(1, uart1_server);
    Create(1, uart1_server);
}
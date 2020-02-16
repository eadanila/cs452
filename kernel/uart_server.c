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
    int s_id = WhoIs("com1");

    for(;;)
    {
        char message[US_REQUEST_LENGTH];
        char reply[US_REPLY_LENGTH];
        message[0] = CHAR_RECEIVED;

        AwaitEvent(EVENT_UART1_RX_INTERRUPT);
        
        // Notify server that a character was received
        Send(s_id, message, US_REQUEST_LENGTH, reply, US_REPLY_LENGTH);
    }
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

void uart1_server(void)
{
    // Register on the name server
    RegisterAs("com1");

    // Notifiers and must be one priority level higher
    int uart1_getc_notifer_tid  =  Create(0, uart1_getc_notifer);
    int uart1_putc_notifer_tid  = Create(0, uart1_putc_notifer);

    RingBuffer put_buffer;
    RingBuffer get_buffer;

    // int unblocked_notifier = -1;
    int queued_getc_task = -1;
    int put_ready = 1;
    int get_ready = 0;

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
                // Only allow authorized notifer to send this message
                if( sender != uart1_getc_notifer_tid ) break;
                get_ready = 1;
                break;

            case PUT_READY:
                if( sender != uart1_putc_notifer_tid ) break;
                put_ready = 1;
                break;
        }

        // Choose if we are going to read or write next
        // and only ever unblock one notifer at a time
        // Currently just prioritize sending
        // TODO Maybe only have one queue to maintain order of get and puts?
        //      if not then maybe interleave gets and puts.
        if(put_ready && put_buffer.size > 0)
        {
            // Unblock notifer so it can start awaiting on
            // CTS low again before sending another byte.
            Reply(uart1_putc_notifer_tid, reply_msg, 0);
            char c = remove_byte(&put_buffer);
            uart_send_byte(UART1, c);

            put_ready = 0;
        }
        else if(get_ready)
        {
            add_byte(&get_buffer, uart_read_byte(UART1));

            // Unblock notifer so it can start awaiting again
            // TODO Should this actually occur before??
            Reply(uart1_getc_notifer_tid, reply_msg, 0);

            get_ready = 0;
        }

        if(queued_getc_task != -1 && get_buffer.size > 0)
        {
            reply_msg[0] = remove_byte(&get_buffer);
            Reply(queued_getc_task, reply_msg, US_REPLY_LENGTH);

            queued_getc_task = -1;
        }
    }
}

void uart2_getc_notifer(void)
{
    int s_id = WhoIs("com2");

    for(;;)
    {
        char message[US_REQUEST_LENGTH];
        char reply[US_REPLY_LENGTH];
        message[0] = CHAR_RECEIVED;

        AwaitEvent(EVENT_UART2_RX_INTERRUPT);
        
        // Notify server that a character was received
        Send(s_id, message, US_REQUEST_LENGTH, reply, US_REPLY_LENGTH);
    }
}

void uart2_putc_notifer(void)
{
    int s_id = WhoIs("com2");

    for(;;)
    {
        char message[US_REQUEST_LENGTH];
        char reply[US_REPLY_LENGTH];
        message[0] = PUT_READY;

        // Get char to print
        Send(s_id, message, US_REQUEST_LENGTH, reply, US_REPLY_LENGTH);
        
        AwaitEvent(EVENT_UART2_TX_INTERRUPT);
    }
}

void uart2_server(void)
{
    // Register on the name server
    RegisterAs("com2");

    // Notifiers and must be one priority level higher
    int uart2_getc_notifer_tid  = Create(0, uart2_getc_notifer);
    int uart2_putc_notifer_tid  = Create(0, uart2_putc_notifer);

    RingBuffer put_buffer;
    RingBuffer get_buffer;

    // int unblocked_notifier = -1;
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
                // Only allow authorized notifer to send this message
                if( sender != uart2_getc_notifer_tid ) break;

                add_byte(&get_buffer, uart_read_byte(UART2));

                // Unblock notifer so it can start awaiting again
                Reply(uart2_getc_notifer_tid, reply_msg, 0);
                break;

            case PUT_READY:
                if( sender != uart2_putc_notifer_tid ) break;

                put_ready = 1;
                break;
        }

        // Choose if we are going to read or write next
        // and only ever unblock one notifer at a time
        // Currently just prioritize sending
        // TODO Maybe only have one queue to maintain order of get and puts?
        //      if not then maybe interleave gets and puts.
        if(put_ready && put_buffer.size > 0)
        {
            char c = remove_byte(&put_buffer);
            uart_send_byte(UART1, c);

            // Unblock notifer
            Reply(uart2_putc_notifer_tid, reply_msg, 0);

            put_ready = 0;
        }

        if(queued_getc_task != -1 && get_buffer.size > 0)
        {
            reply_msg[0] = remove_byte(&get_buffer);
            Reply(queued_getc_task, reply_msg, US_REPLY_LENGTH);

            queued_getc_task = -1;
        }
    }
}

void create_uart_servers(void)
{
    Create(1, uart1_server);
    Create(1, uart1_server);
}
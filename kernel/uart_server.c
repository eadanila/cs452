#include "uart_server.h"
#include "bwio.h"
#include "syscall.h"
#include "logging.h"
#include "name_server.h"
#include "uart.h"
#include "await.h"
#include "string_utility.h"

#define GET_CHAR 1
#define PUT_CHAR 2
#define PUT_COMMAND 3
#define CHAR_RECEIVED 4
#define PUT_READY 5
#define NOTIFIER_STARTUP 6

// First byte is the channel, Second is the operation
#define US_REQUEST_LENGTH 3
#define US_REPLY_LENGTH 1

#define BUFFER_SIZE 10240

// A circular buffer of chars
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

    char message[1];
    char reply[US_REPLY_LENGTH];

    message[0] = GET_CHAR;

    // if tid is not the task id of an existing task.
    if(Send(tid, message, 1, reply, US_REPLY_LENGTH) == -1) 
        return INVALID_UART_SERVER;

    return reply[0];
}

int Putc(int tid, int channel, char ch)
{
    // Currently channel is ignored
    // assert(channel == COM1 || channel == COM2);

    char message[2];
    char reply[US_REPLY_LENGTH];

    message[0] = PUT_CHAR;
    message[1] = ch;

    // if tid is not the task id of an existing task.
    if(Send(tid, message, 2, reply, US_REPLY_LENGTH) == -1) 
        return INVALID_UART_SERVER;

    return 0;
}

int PutCommand(int tid, int channel, char ch1, char ch2)
{
    // Currently channel is ignored
    // assert(channel == COM1 || channel == COM2);

    char message[3];
    char reply[US_REPLY_LENGTH];

    message[0] = PUT_COMMAND;
    message[1] = ch1;
    message[2] = ch2;

    // if tid is not the task id of an existing task.
    if(Send(tid, message, 3, reply, US_REPLY_LENGTH) == -1) 
        return INVALID_UART_SERVER;

    return 0;
}

void uart1_getc_notifer(void)
{
    int s_id = WhoIs("com1");

    char message[US_REQUEST_LENGTH];
    char reply[US_REPLY_LENGTH];
    message[0] = NOTIFIER_STARTUP;

    Send(s_id, message, US_REQUEST_LENGTH, reply, US_REPLY_LENGTH);
    message[0] = CHAR_RECEIVED;

    for(;;)
    {
        message[1] = AwaitEvent(EVENT_UART1_RX_INTERRUPT);

        // Notify server that a character was received
        Send(s_id, message, US_REQUEST_LENGTH, reply, US_REPLY_LENGTH);
    }
}

void uart1_putc_notifer(void)
{
    int s_id = WhoIs("com1");

    char message[US_REQUEST_LENGTH];
    char reply[US_REPLY_LENGTH];
    message[0] = PUT_READY;

    for(;;)
    {
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
    // Both notifiers start off initially blocked
    // and only one is ever unblocked at a time.
    int uart1_getc_notifer_tid  =  Create(0, uart1_getc_notifer);
    int uart1_putc_notifer_tid  = Create(0, uart1_putc_notifer);

    RingBuffer put_buffer;
    // TODO, replace with a single character since only one task can call Getc at a time
    RingBuffer get_buffer;

    int queued_getc_task = -1;
    int put_ready = 1;

    int receiving = 0;

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
                // print("GET_CALLED");
                
                if(get_buffer.size > 0) // Character already received, reply immediately
                {
                    reply_msg[0] = remove_byte(&get_buffer);
                    Reply(sender, reply_msg, US_REPLY_LENGTH);
                }
                else if(queued_getc_task == -1) // Block until char received
                {
                    queued_getc_task = sender;
                }
                else // Another task is already waiting for a char, return error code
                {
                    reply_msg[0] = OTHER_TASK_QUEUED;
                    Reply(sender, reply_msg, US_REPLY_LENGTH);
                }
                break;

            case PUT_CHAR:
                // print("PUT_CALLED ");
                // Put char onto queue to be sent asap and reply immediately
                add_byte(&put_buffer, msg[1]);
                Reply(sender, reply_msg, US_REPLY_LENGTH);
                break;

            case PUT_COMMAND:
                // print("PUT_COMMAND_CALLED ");
                // Put both chars onto queue to be sent asap and reply immediately
                add_byte(&put_buffer, msg[1]);
                add_byte(&put_buffer, msg[2]);
                Reply(sender, reply_msg, US_REPLY_LENGTH);
                break;

            case CHAR_RECEIVED:
                // print("CHAR_RECEIVED ");
                // Only allow authorized notifer to send this message
                if( sender != uart1_getc_notifer_tid ) break;

                char read_byte = msg[1];

                receiving--;
                assert(receiving >= 0);

                // If no more bytes are expected, uart is ready to putc again.
                // Otherwise wake getc notifer again.
                if(receiving == 0)
                {
                    add_byte(&get_buffer, read_byte);
                    // We assume that after our last byte has been received, we are 
                    // immediately ready to transmit since UART1 is half-duplex.
                    put_ready = 1;
                }
                else
                {
                    add_byte(&get_buffer, read_byte);
                    Reply(uart1_getc_notifer_tid, reply_msg, 0);
                }

                break;

            case PUT_READY:
                // print("PUT_READY ");
                // Sent by the putc notifier
                if( sender != uart1_putc_notifer_tid ) break;
                put_ready = 1;
                break;

            case NOTIFIER_STARTUP:
                // print("NOTIFIER_STARTUP ");
                // Do nothing, this message was sent to make the 
                // getc notifier initially blocked.
                break;

            default:
                assert(0);
                break;
        }

        // Choose if we are going to read or write next
        // and only ever unblock one notifer at a time
        // Currently just prioritize sending
        // TODO Maybe only have one queue to maintain order of get and puts?
        //      if not then maybe interleave gets and puts.
        if(put_ready && put_buffer.size > 0 && receiving == 0)
        {
            char c = remove_byte(&put_buffer);

            // If the character being put triggers a sensor dump,
            // set receiving to the number of bytes expected,
            // leave the putc notifier blocked, and unblock the getc notifier.
            if((c >= MULTI_SENSOR_DUMP_OFFSET && c <= MULTI_SENSOR_DUMP_OFFSET + 5) ||
               (c >= SINGLE_SENSOR_DUMP_OFFSET && c <= SINGLE_SENSOR_DUMP_OFFSET + 5))
            {
                if(c >= SINGLE_SENSOR_DUMP_OFFSET)
                    receiving = c - SINGLE_SENSOR_DUMP_OFFSET;
                else
                    receiving = c - MULTI_SENSOR_DUMP_OFFSET;

                receiving *= 2;

                Reply(uart1_getc_notifer_tid, reply_msg, 0);
                // Reply(uart1_putc_notifer_tid, reply_msg, 0);
                uart_send_byte(UART1, c);
            }
            else
            {
                // Unblock notifer so it can start awaiting on
                // CTS low again before sending another byte.
                Reply(uart1_putc_notifer_tid, reply_msg, 0);
                uart_send_byte(UART1, c);
            }

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

void uart2_getc_notifer(void)
{
    int s_id = WhoIs("com2");

    for(;;)
    {
        char message[US_REQUEST_LENGTH];
        char reply[US_REPLY_LENGTH];
        message[0] = CHAR_RECEIVED;

        message[1] = AwaitEvent(EVENT_UART2_RX_INTERRUPT);
        
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
    // Both notifiers may be unblocked at a time.
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
                if(get_buffer.size > 0) // Character already received, reply immediately
                {
                    reply_msg[0] = remove_byte(&get_buffer);
                    Reply(sender, reply_msg, US_REPLY_LENGTH);
                }
                else if(queued_getc_task == -1) // Block until char received
                {
                    queued_getc_task = sender;
                }
                else // Another task is already waiting for a char, return error code
                {
                    reply_msg[0] = OTHER_TASK_QUEUED;
                    Reply(sender, reply_msg, US_REPLY_LENGTH);
                }
                break;

            case PUT_CHAR:
                add_byte(&put_buffer, msg[1]);
                Reply(sender, reply_msg, US_REPLY_LENGTH);
                break;

            case PUT_COMMAND:
                // Currently not supported.
                assert(0);
                break;

            case CHAR_RECEIVED:
                // Only allow authorized notifer to send this message
                if( sender != uart2_getc_notifer_tid ) break;

                add_byte(&get_buffer, msg[1]);

                // Unblock notifer so it can start awaiting again
                Reply(uart2_getc_notifer_tid, reply_msg, 0);
                break;

            case PUT_READY:
                if( sender != uart2_putc_notifer_tid ) break;

                put_ready = 1;
                break;
        }

        if(put_ready && put_buffer.size > 0)
        {
            char c = remove_byte(&put_buffer);
            uart_send_byte(UART2, c);

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
    // Create(1, uart1_noqueue_server);
    Create(1, uart1_server);
    Create(1, uart2_server);
}
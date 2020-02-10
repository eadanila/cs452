#include "clock_server.h"
#include "await.h"
#include "syscall.h"
#include "name_server.h"
#include "logging.h"

#define CS_REQUEST_LENGTH 5 // 1 char for operation type, 4 chars for int paramter
#define REPLY_LENGTH 4 // 4 chars for int parameter

// Codes corresponding to possible operations to send the clock server
#define TICK_OCCURED 127
#define TIME_ELAPSED 1
#define DELAY 2
#define DELAY_UNTIL 3

// Initialize SortedList structure.
// Importantly, initializes its internal linked list of free ListNodes
void init_sorted_list(SortedList* l)
{
    for(int i = 0; i != MAX_TASKS_ALLOWED - 1; i++) l->nodes[i].next = &l->nodes[i+1];
    l->nodes[MAX_TASKS_ALLOWED - 1].next = 0;

    l->free = &l->nodes[0];
    l->list = 0;
}

// Add a tid associated with a time to a SortedList structure.
// The tid and time pair is inserted such that the SortedList 
// remains sorted in increasing order by time.
int add_item(SortedList* l, int tid, int time)
{
    // Error if list is full! Should never happen.
    if(l->free == 0) return 1;

    // If list is empty
    if(l->list == 0) 
    {
        l->list = l->free;
        l->free = l->free->next;
        l->list->next = 0;// TODO Is this being done in other linked list implementations?

        l->list->tid = tid;
        l->list->time = time;

        return 0;
    }

    // NOTE: If a time is equal to other nodes times, it is placed AFTER those times

    // If new item should be at the front of the list
    if(time < l->list->time)
    {
        ListNode* new_node = l->free;
        l->free = l->free->next;

        new_node->next = l->list;
        l->list = new_node;

        new_node->tid = tid;
        new_node->time = time;

        return 0;
    }

    // The new node will now be guaranteed to be past the front of list

    // Scan through list to find where item should be (sorting smallest time to largest)
    ListNode* scanner = l->list;

    while(scanner->next && time >= scanner->next->time) scanner = scanner->next;

    // "scanner" should now point to the element in front of where the new node should go

    ListNode* new_node = l->free;
    l->free = l->free->next;

    new_node->next = scanner->next;
    scanner->next = new_node;

    new_node->tid = tid;
    new_node->time = time;

    return 0;
}

// Return the tid at the front of the sorted list
int peek_front_tid(SortedList *l)
{
    if(l->list) return l->list->tid;

    return -1;
}

// Return the time associated with the tid at the front of the sorted list
int peek_front_time(SortedList *l)
{
    if(l->list) return l->list->time;

    return -1;
}

// Remove the tid and time pair at the front of the sorted list
void remove_front(SortedList *l)
{
    assert(l->list != 0);
    ListNode* next_free = l->free;
    l->free = l->list;
    l->list = l->list->next;
    l->free->next = next_free;
}

int is_empty(SortedList *l)
{
    return l->list == 0;
}

// Store an int into a 4 byte long char array
void pack_int(int i, char* buffer)
{
    // int* b = (int*) buffer;
    // *b = i;
    
    unsigned char* b = (unsigned char*) buffer;
    for(int j = 3; j >= 0; j--)
    {
        b[j] = i & 0xff;
        i = i >> 8;
    }
}

// Extract an int from a 4 byte long char array create by pack_int
int unpack_int(char* buffer)
{
    // int* b = (int*) buffer;
    // return *b;

    unsigned char* b = (unsigned char*) buffer;
    int r = 0;
    for(int i = 0; i != 3; i++)
    {
        r |= (unsigned int)b[i];
        r = r << 8;
    }
    r |= (unsigned int)b[3];

    return r;
}

int Time(int tid)
{
    char message[CS_REQUEST_LENGTH];
    char reply[REPLY_LENGTH];

    message[0] = TIME_ELAPSED;

    // if tid is not the task id of an existing task.
    if(Send(tid, message, CS_REQUEST_LENGTH, reply, REPLY_LENGTH) == -1) return INVALID_CLOCK_SERVER_TASK;

    return unpack_int(reply);
}

int Delay(int tid, int ticks)
{
    char message[CS_REQUEST_LENGTH];
    char reply[REPLY_LENGTH];

    message[0] = DELAY;
    pack_int(ticks, message + 1);

    // if tid is not the task id of an existing task.
    if(Send(tid, message, CS_REQUEST_LENGTH, reply, REPLY_LENGTH) == -1) 
        return INVALID_CLOCK_SERVER_TASK;

    return unpack_int(reply);
}

int DelayUntil(int tid, int ticks)
{
    char message[CS_REQUEST_LENGTH];
    char reply[REPLY_LENGTH];

    message[0] = DELAY_UNTIL;
    pack_int(ticks, message + 1);

    // if tid is not the task id of an existing task.
    if(Send(tid, message, CS_REQUEST_LENGTH, reply, REPLY_LENGTH) == -1) 
        return INVALID_CLOCK_SERVER_TASK;

    return unpack_int(reply);
}

void clock_notifier(void)
{
    // Get clock server tid
    int cs_id = WhoIs("clock_server");
    
    for(;;)
    {
        // Notify the clock server that a tick occured every time 
        // timer1 interupts (every 10 ms)
        AwaitEvent(EVENT_TIMER1_INTERRUPT);

        char message[CS_REQUEST_LENGTH];
        char reply[REPLY_LENGTH];

        message[0] = TICK_OCCURED;

        Send(cs_id, message, CS_REQUEST_LENGTH, reply, REPLY_LENGTH);
    }
}

void clock_server(void)
{
    RegisterAs("clock_server");

    // int n_id = 
    Create(0, clock_notifier);

    // intialize the ticks elapsed since the creation of the clock server
    // and the sorted list of tids currently delayed.
    int ticks_elapsed = 0;
    SortedList waiting;
    init_sorted_list(&waiting);

    int sender;
    char msg[CS_REQUEST_LENGTH];
    char reply_msg[REPLY_LENGTH];
    int arg = 0;

    for(;;)
    {
        // Receive clock server request
        Receive(&sender, msg, CS_REQUEST_LENGTH);

        // Execute the command encoded in the request
        switch(msg[0])
        {
            // Clock notifier is notifying the server that a tick has occured.
            // Reply to unblock it.
            case TICK_OCCURED:
                ticks_elapsed++;
                Reply(sender, reply_msg, 0); // Reply message content doesn't matter
                break;

            // Request has been made by Time().
            // Reply with the ticks elapsed since the clock server's creation.
            case TIME_ELAPSED:
                pack_int(ticks_elapsed, reply_msg);
                Reply(sender, reply_msg, REPLY_LENGTH);
                break;

            // Request has been made by Delay().
            // Reply immediatly if the delay given was negative. Otherwise, add the tid
            // to the SortedList "waiting" to be unblocked when the delay time has elapsed.
            // The time associated with the tid is the current time + the delay value given,
            // this is the time that they're expected to delay until.
            case DELAY:
                arg = unpack_int(msg + 1);
                if(arg < 0)
                {
                    pack_int(NEGATIVE_DELAY, reply_msg);
                    Reply(sender, reply_msg, REPLY_LENGTH);
                }
                else
                {
                    // for(int i = 0; i != 4; i++) print("%d ", msg[1+i]);
                    int r = add_item(&waiting, sender, ticks_elapsed + arg);
                    assert(r == 0); 
                    r++; // suppress pedantic
                    // Cannot put send inside of assert because asserts evaluate 
                    // to empty braces when in release
                }
                break;

            // Request has been made by DelayUntil().
            // Reply immediatly if the given time to delay until was negative. Otherwise, add the tid
            // to the SortedList "waiting" to be unblocked after the given time has passed.
            case DELAY_UNTIL:
                arg = unpack_int(msg + 1);
                if(arg < 0)
                {
                    pack_int(NEGATIVE_DELAY, reply_msg);
                    Reply(sender, reply_msg, REPLY_LENGTH);
                }
                else
                {
                    int r = add_item(&waiting, sender, arg);
                    assert(r == 0);
                    r++; // suppress pedantic
                }
                break;
        }

        // Unblock all tasks whos time that they're expected to delay until has passed.
        // Remove them from the waiting list once they are unblocked.
        while(!is_empty(&waiting) && peek_front_time(&waiting) <= ticks_elapsed)
        {
            pack_int(ticks_elapsed, reply_msg);
            Reply(peek_front_tid(&waiting), reply_msg, REPLY_LENGTH);
            
            remove_front(&waiting);
        }
    }
}

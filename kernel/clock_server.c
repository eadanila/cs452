#include "clock_server.h"
#include "await.h"
#include "syscall.h"
#include "name_server.h"
#include "logging.h"

// 1 char for operation type, 4 chars for int paramter
#define REQUEST_LENGTH 5 
#define REPLY_LENGTH 4

#define TICK_OCCURED 127
#define TIME_ELAPSED 1
#define DELAY 2
#define DELAY_UNTIL 3

typedef struct list_node ListNode;
typedef struct sorted_list SortedList;

struct list_node
{
    int tid;
    int time; // In ticks

    ListNode* next;
};


struct sorted_list
{
    ListNode nodes[MAX_TASKS_ALLOWED];

    ListNode *free;
    ListNode *list;
};

void init_sorted_list(SortedList* l)
{
    for(int i = 0; i != MAX_TASKS_ALLOWED - 1; i++) l->nodes[i].next = &l->nodes[i+1];
    l->nodes[MAX_TASKS_ALLOWED - 1].next = 0;

    l->free = &l->nodes[0];
    l->list = 0;
}

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

    // If new item should be at the front of the list
    // If time is equal to other nodes times, place it AFTER those times
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

    // New node will now be guaranteed to be past the front of list

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

int peek_front_tid(SortedList *l)
{
    if(l->list) return l->list->tid;

    return -1;
}

int peek_front_time(SortedList *l)
{
    if(l->list) return l->list->time;

    return -1;
}

void remove_front(SortedList *l)
{
    ListNode* next_free = l->free;
    l->free = l->list;
    l->list = l->list->next;
    l->free->next = next_free;
}

int is_empty(SortedList *l)
{
    return l->list == 0;
}

// Store an int into a 4 long char array
void pack_int(int i, char* buffer)
{
    // *((int*)buffer) = i;
    
    unsigned char* b = (unsigned char*) buffer;
    for(int j = 3; j >= 0; j--)
    {
        b[j] = i & 0xff;
        i = i >> 8;
    }
}

// Extract an int from a 4 long char array create by pack_int
int unpack_int(char* buffer)
{
    // return *((int*) buffer);

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
    char message[REQUEST_LENGTH];
    char reply[REPLY_LENGTH];

    message[0] = TIME_ELAPSED;

    Send(tid, message, REQUEST_LENGTH, reply, REPLY_LENGTH);

    return 0;
}

int Delay(int tid, int ticks)
{
    char message[REQUEST_LENGTH];
    char reply[REPLY_LENGTH];

    message[0] = DELAY;
    pack_int(ticks, message + 1);
    // Could replace pack_int with the following line, but need to confirm 
    // if int pointers point to most or least significant byte.
    // *((int*)(message + 1)) = ticks;

    Send(tid, message, REQUEST_LENGTH, reply, REPLY_LENGTH);

    return 0;
}

int DelayUntil(int tid, int ticks)
{
    char message[REQUEST_LENGTH];
    char reply[REPLY_LENGTH];

    message[0] = DELAY_UNTIL;
    pack_int(ticks, message + 1);

    Send(tid, message, REQUEST_LENGTH, reply, REPLY_LENGTH);

    return 0;
}

void clock_notifier(void)
{
    int cs_id = WhoIs("clock_server");
    
    for(;;)
    {
        AwaitEvent(EVENT_TIMER1_INTERRUPT);

        char message[REQUEST_LENGTH];
        char reply[REPLY_LENGTH];

        message[0] = TICK_OCCURED;

        Send(cs_id, message, REQUEST_LENGTH, reply, REPLY_LENGTH);
    }
}

void clock_server(void)
{
    RegisterAs("clock_server");

    // int n_id = 
    Create(0, clock_notifier);

    int ticks_elapsed = 0;
    SortedList waiting;
    init_sorted_list(&waiting);

    int sender;
    char msg[REQUEST_LENGTH];
    char reply_msg[REPLY_LENGTH];
    int arg = 0;

    for(;;)
    {
        Receive(&sender, msg, REQUEST_LENGTH);

        switch(msg[0])
        {
            case TICK_OCCURED:
                ticks_elapsed++;
                Reply(sender, reply_msg, REPLY_LENGTH); // Reply message content doesn't matter
                break;

            case TIME_ELAPSED:
                pack_int(ticks_elapsed, reply_msg);
                Reply(sender, reply_msg, REPLY_LENGTH);
                break;

            case DELAY:
                if(arg < 0)
                {
                    pack_int(NEGATIVE_DELAY, reply_msg);
                    Reply(sender, reply_msg, REPLY_LENGTH);
                }
                else
                {
                    // for(int i = 0; i != 4; i++) print("%d ", msg[1+i]);
                    arg = unpack_int(msg + 1);
                    add_item(&waiting, sender, ticks_elapsed + arg);
                }
                break;

            case DELAY_UNTIL:
                if(arg < 0)
                {
                    pack_int(NEGATIVE_DELAY, reply_msg);
                    Reply(sender, reply_msg, REPLY_LENGTH);
                }
                else
                {
                    arg = unpack_int(msg + 1);
                    add_item(&waiting, sender, arg);
                }
                break;
        }

        while(!is_empty(&waiting) && peek_front_time(&waiting) <= ticks_elapsed)
        {
            pack_int(ticks_elapsed, reply_msg);
            Reply(peek_front_tid(&waiting), reply_msg, REPLY_LENGTH);
            
            remove_front(&waiting);
        }
    }
}
#ifndef CLOCK_SERVER_H
#define CLOCK_SERVER_H

#define INVALID_CLOCK_SERVER_TASK -1
#define NEGATIVE_DELAY -2

#include "constants.h"

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

int Time(int tid);
int Delay(int tid, int ticks);
int DelayUntil(int tid, int ticks);

void clock_notifier(void);
void clock_server(void);

#endif


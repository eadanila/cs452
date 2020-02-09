#define INVALID_CLOCK_SERVER_TASK -1
#define NEGATIVE_DELAY -2

int Time(int tid);
int Delay(int tid, int ticks);
int DelayUntil(int tid, int ticks);

void clock_notifier(void);
void clock_server(void);
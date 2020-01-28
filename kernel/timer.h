#define DEBUG_TIMER_HIGH (volatile unsigned int *) 0x80810064
#define DEBUG_TIMER_LOW  (volatile unsigned int *) 0x80810060
#define DEBUG_TIMER_CTRL 0x100 // 1 << 8
#define DEBUG_TIMER_HIGH_MASK 0xFF // first 8 bits

void read_debug_timer(int *high, int *low);
void stop_debug_timer(void);
void start_debug_timer(void);


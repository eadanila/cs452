#include "train_timing.h"
#include "logging.h"
#include "bwio.h"
#include "uart.h"
#include "interrupt.h"
#include "ts7200.h"
#include "timer.h"
#include "sensors.h"

#define TEST_NAME "outermost_loop_clockwise"
#define TRACK 'A'
#define TRAIN_ID 79
#define TRAIN_SPEED 14

#define STOPPING_TEST 0
#define STOPPING_SENSOR 58 // D11=58 for track A
#define LOOPS 10
// A is LEFT track

// Tests needed:
// center_loop

#define PRINTS_PER_LINE 8
#define DISCARD_HITS 5

static volatile int all_sensor_states[128];

int seconds_to_tics(float seconds)
{
	return 983040*seconds;
}

void busy_delay(int ticks)
{
    int start_time = read_debug_timer();
    while(read_debug_timer() < (unsigned int)(start_time + ticks));
}

void timing()
{
    // Initialize COM2
    bwsetspeed(COM2, 115200);
    bwsetfifo(COM2, OFF);

    bwsetspeed(COM1, 2400);
    bwsetfifo(COM1, OFF);
    bwtraininitialize(COM1);

    clear_vic();
    disable_interrupt(INTERRUPT_TC3UI);
    disable_interrupt(INTERRUPT_TC1UI);
    disable_interrupt(INTERRUPT_UART1RXINTR1);
    disable_interrupt(INTERRUPT_UART1TXINTR1);
    disable_interrupt(INTERRUPT_UART1);
    disable_interrupt(INTERRUPT_UART2RXINTR2);
    disable_interrupt(INTERRUPT_UART2TXINTR2);
    disable_interrupt(INTERRUPT_UART2);

    stop_debug_timer();
    start_debug_timer();

    // // Clear RX interupts by reading bytes
    for(volatile int i = 0; i != 1000; i++) uart_read_byte(UART1);

    int* line;
	line = (int *)( UART1_BASE + VAL_OFFSET );
    *line &= ~0xF; // 0b1111

    // set_timer_mode(TIMER_TC3, 0);
    // enable_timer(TIMER_TC3);
    // set_timer_clock(TIMER_TC3, 1);

    // // set_timer_clock(TIMER_TC1, 1); // 508.4KHz
    // // set_timer_mode(TIMER_TC1, 1);
    // set_timer_load_value(TIMER_TC3, __INT_MAX__); // 10ms @ 508.4KHz => 5084 clock cycles

    bwputc(COM1, 96);

    busy_delay(seconds_to_tics(1));

    // Set train speed to 0 first
    bwputc(COM1, 0);
    bwputc(COM1, TRAIN_ID);

    busy_delay(seconds_to_tics(5));

    bwputc(COM1, TRAIN_SPEED);
    bwputc(COM1, TRAIN_ID);

    busy_delay(seconds_to_tics(0.25));

    for(int i = 0; i != 128; i++) all_sensor_states[i] = 0;

    char sensors[SENSOR_COUNT];
    char newly_triggered[SENSOR_COUNT];

    int printed_on_line = 0;
    int discarded = 0;
    int first_sensor = -1;
    int loops = -1;

    if(STOPPING_TEST)
    {
        print("\n\r");
        print("TEST: %s \n\r", TEST_NAME);
        print("TRACK: %c \n\r", TRACK);
        print("TRAIN_ID: %d \n\r", TRAIN_ID);
        print("SPEED: %d \n\r", TRAIN_SPEED);
        print("STOPPING_TEST \n\r");
        print("STOPPING SENSOR %d \n\r", STOPPING_SENSOR);
    }
    else
    {
        print("\n\r");
        print("TEST: %s \n\r", TEST_NAME);
        print("TRACK: %c \n\r", TRACK);
        print("TRAIN_ID: %d \n\r", TRAIN_ID);
        print("SPEED: %d \n\r", TRAIN_SPEED);
    }

    for(;;)
    {
        char sensor_dump[10];
        int time_hit;

        bwputc(COM1, 133);
        
        time_hit = read_debug_timer();
        for(int i = 0; i != 10; i++)
        {
            sensor_dump[i] = bwgetc(COM1);
        } 

        parse_sensors(sensor_dump, sensors, newly_triggered);

        // Only one sensor hit at a time in these tests
        int sensor_hit = 0;

        for(; sensor_hit != SENSOR_COUNT; sensor_hit++) 
        {
            if(newly_triggered[sensor_hit]) break;
        }

        // No new sensor hit
        if(sensor_hit == SENSOR_COUNT) sensor_hit = -1;

        if(sensor_hit > 0)
        {
            if(STOPPING_TEST && sensor_hit == STOPPING_SENSOR)
            {
                bwputc(COM1, 0);
                bwputc(COM1, TRAIN_ID);
                bwgetc(COM2);
            }
            else
            {
                if(discarded > DISCARD_HITS)
                {
                    if(first_sensor == -1) first_sensor = sensor_hit;
                    else if(sensor_hit == first_sensor)
                    {
                        loops++;
                        if(loops >= LOOPS)
                        {
                        print("\n\rFinished\n\r");
                        break;
                        } 
                    }

                    char sensor_char;
                    int sensor_count;

                    sensor_name(sensor_hit, &sensor_char, &sensor_count);

                    // Name, milliseconds, index, debug timer ticks
                    print("%c%d %d %d %d, ", sensor_char, sensor_count, time_hit/983, sensor_hit, time_hit);

                    printed_on_line  = (printed_on_line + 1) % PRINTS_PER_LINE;
                    if(printed_on_line == 0 ) print("\n\r");
                }
                else
                {
                    discarded++;
                }
            }
        }
    }
}
#include "terminal.h"
#include "name_server.h"
#include "uart_server.h"
#include <bwio.h>
#include "print.h"
#include "string_utility.h"
#include "syscall.h"
#include "logging.h"
#include "clock_server.h"
#include "tc_server.h"
#include "timer.h"
#include "track_display.h"

#define COMMAND_PRINT_HEIGHT 35
#define SWITCH_PRINT_HEIGHT 9
#define SENSOR_PRINT_HEIGHT 7
#define TIME_PRINT_HEIGHT 5
#define INITIALIZATION_PRINT_HEIGHT 3
#define SPINNER_PRINT_HEIGHT 3
#define TRACK_PRINT_HEIGHT 13
#define TRACK_PRINT_COL 2

#define MAX_COMMAND_LEN 127
#define MAX_TPRINT_SIZE 1024

#define TERMINAL_TICK_NOTIFIER_DELAY 5
#define TERMINAL_NOTIFIER_PRIORITY 4

// The following variables are used by functions only called by the terminal task.
// Although "global", these variables are accessed by no other task
// than the terminal task and thus do not violate memory exclusivity.
// These exist to avoid constantly passing large amounts
// of variables to functions only called by the terminal task.
char command[MAX_COMMAND_LEN + 1];
int command_len;
int last_command_len;

int pid;
int com1_id;
int tcid;

int track_initialized;

// TEMP 
// Variables used to hold senor states, and a queue of the last 16
// triggered sensors. Declared volatile to avoid the compiler using undefined 
// functions to optimize operations done on them.
volatile int sensor_states[16];
volatile int all_sensor_states[128];

// Variables used to store switches triggered and changed. 
// In the future this responsibility may move the tc_server.
int switches[SWITCH_COUNT];
char switch_states[256];
int switch_updated;

TrackView track_a_curved_view;
TrackView track_a_straight_view;
TrackView track_b_curved_view;
TrackView track_b_straight_view;
char active_track;

void TPrint(int tid, char* str, ... )
{
	va_list va;
	va_start(va, str);

    char message[MAX_TPRINT_SIZE];
    char reply[1];

	// If formatted string is longer than MAX_TPRINT_SIZE,
	// it should be truncated.
	_format_string(message, MAX_TPRINT_SIZE, str, va);

    Send(tid, message, MAX_TPRINT_SIZE, reply, 0);

	va_end(va);
}

void command_append(char c)
{
	// Maximum command size
	if(command_len == MAX_COMMAND_LEN - 1) return;

	// TODO Bound Check
	command[command_len] = c;
	command_len++;
	command[command_len] = 0;
}

void command_popback()
{
	if(command_len == 0) return;
	// TODO Bound Check
	command_len--;
	command[command_len] = 0;
}

void command_clear()
{ 
	command[0] = 0; 
	command_len = 0;
	last_command_len = 0;
	
	// Clear the printed output
	char cleared[MAX_COMMAND_LEN];
	for(int i = 0; i < MAX_COMMAND_LEN; i++) cleared[i] = ' ';
	cleared[MAX_COMMAND_LEN - 1] = 0;

	MoveCursor(pid, 0, COMMAND_PRINT_HEIGHT);
	Print(pid, cleared);
}

void print_command()
{
	if(command_len == last_command_len) return;
	else if(command_len > last_command_len)
	{
		// Make cursor go to bottom left.
		MoveCursor(pid, command_len, COMMAND_PRINT_HEIGHT);
		Putc(pid, COM2, command[command_len-1]);
	}
	else
	{
		// Make cursor go to bottom left.
		MoveCursor(pid, 1+command_len, COMMAND_PRINT_HEIGHT);
		Putc(pid, COM2, ' ');
	}
}

// Increment *command past prefix if it matches.
// Returns if the command contained the prefix.
int is_command(char* prefix, char** command)
{
	int prefix_len = _strlen(prefix);
	int r = _strsim(prefix, *command) == prefix_len;
	if(r) (*command) += prefix_len;
	return r;
}

// Increment *command past the argument if the argument matches.
// Returns if the command immediatly contained the argument.
int is_arg(char* arg, char** command)
{
	char* command_p = *command;

	// Empty string provided, invalid argument
	if(*command_p == 0) return 0;

	// Skip whitespace before
	while(*command_p && *command_p == ' ') command_p++;

	// End of string, invalid argument
	if(*command_p == 0) return 0;

	if(is_command(arg, &command_p))
	{
		*command = command_p;
		return 1;
	}
	
	return 0;
}

// Increment *command if a valid int is parsed, set *command = 0 otherwise.
// Return the parsed int.
int parse_int(char** command)
{
	// Empty string provided, invalid int string
	if(**command == 0)
	{
		*command = 0;
		return 0;
	}

	// Skip whitespace before
	while(**command && **command == ' ') (*command)++;
	
	// Store where int would start if the following is a valid int
	char* command_int = *command;

	// Check if the following non whitespace is a valid int
	if(**command == '-') (*command)++;

	// If string is only a "-", invalid int string
	if(**command == 0 || **command == ' ')
	{
		*command = 0;
		return 0;
	}

	while(**command && **command != ' ')
	{
		// non-digit encountered, invalid int string
		if(**command > '9' || **command < '0') 
		{
			*command = 0;
			return 0;
		}
		
		(*command)++;
	} 

	// *command now stores one space after the end of the string.
	return stoi(command_int);
}

void print_invalid_switch_track()
{
	MoveCursor(pid, 0, COMMAND_PRINT_HEIGHT - 1);
	Print(pid, "Ensure all turnouts are set to straight before switching tracks!\n\r");
}

void print_initialization_incomplete()
{
	MoveCursor(pid, 0, COMMAND_PRINT_HEIGHT - 1);
	Print(pid, "Initialization incomplete, command ignored!\n\r");
}

void print_invalid_argument()
{
	MoveCursor(pid, 0, COMMAND_PRINT_HEIGHT - 1);
	Print(pid, "Invalid argument provided!\n\r");
}

void print_invalid_command()
{
	MoveCursor(pid, 0, COMMAND_PRINT_HEIGHT - 1);
	Print(pid, "Invalid command provided!\n\r");
}

void clear_invalid_message()
{
	MoveCursor(pid, 0, COMMAND_PRINT_HEIGHT - 1);
	Print(pid, "                                                                            \n\r");
}

void print_time(int curr_time)
{
	// Create time string
	float seconds_float = (float)curr_time / 100.0f;
	int tenths = ((int)(seconds_float*10))%10;
	int seconds = ((int)seconds_float)%60;
	int minutes = seconds_float/60;

	char tenths_str[8];
	char seconds_str[8];
	char minutes_str[8];

	itos(tenths, tenths_str);
	itos(seconds, seconds_str);
	itos(minutes, minutes_str);

	// If seconds is 1 digit, add zero in front
	if(_strlen(seconds_str) == 1)
	{
		seconds_str[2] = 0;
		seconds_str[1] = seconds_str[0];
		seconds_str[0] = '0';
	}

	Print(pid, "%s:%s.%s\n\r\n\r", minutes_str, seconds_str, tenths_str);
}

void process_command()
{	
	if (command_len == 0) return;

	char* command_p = command;

	// All commands past this point (q, tr, rv, sw) are dangerous if executed
	// during initialization.
	if(!track_initialized)
	{
		print_initialization_incomplete();
		return;
	} 

	if (is_command("q", &command_p)) 
	{
		Shutdown();
	}
	else if (is_command("tr", &command_p)) 
	{
		int t_number = parse_int(&command_p);
		int t_speed = parse_int(&command_p);

		if(command_p == 0) print_invalid_argument();

		SetSpeed(tcid, t_number, t_speed);
	}
	else if (is_command("rv", &command_p)) 
	{
		int t_number = parse_int(&command_p);
		if(command_p == 0) print_invalid_argument();

		Reverse(tcid, t_number);
	}
	else if (is_command("sw", &command_p)) 
	{
		int s_number = parse_int(&command_p);

		if(command_p == 0) print_invalid_argument();

		if(is_arg("C", &command_p))
		{
			assert(switch_states[s_number]);
			SwitchTrackAsync(tcid, s_number, CURVED);
			switch_states[s_number] = 'C';

			if(active_track == TRACK_A) toggle_switch(pid, track_a_curved_view, s_number);
			else if(active_track == TRACK_B) toggle_switch(pid, track_b_curved_view, s_number);
		} 
		else if(is_arg("S", &command_p))
		{
			assert(switch_states[s_number]);
			SwitchTrackAsync(tcid, s_number, STRAIGHT);
			switch_states[s_number] = 'S';

			if(active_track == TRACK_A) toggle_switch(pid, track_a_straight_view, s_number);
			else if(active_track == TRACK_B) toggle_switch(pid, track_b_straight_view, s_number);
		} 
		else print_invalid_argument();

		switch_updated = 1;
	}
	else if (is_command("st", &command_p)) 
	{
		for(int i = 0; i != 256; i++)
		{
			if(!(switch_states[i] == 0 || switch_states[i] == 'S'))
			{
				print_invalid_switch_track();
				return;
			}
		}

		if(is_arg("A", &command_p))
		{
			active_track = TRACK_A;
			print_track(pid, track_a_straight_view.data, TRACK_PRINT_COL, TRACK_PRINT_HEIGHT);
		} 
		else if(is_arg("B", &command_p))
		{
			active_track = TRACK_B;
			print_track(pid, track_b_straight_view.data, TRACK_PRINT_COL, TRACK_PRINT_HEIGHT);
		} 
		else print_invalid_argument();
	}
	else
	{
		print_invalid_command();
	}
}

// Continously spins for input from the UART2 server and sends it 
// to the terminal task.
void input_notifier()
{
	int pid = WhoIs("com2");
	int t_id = WhoIs("terminal");

    char message[1];
    char reply[1];

    for(;;)
    {
    	message[0] = Getc(pid, COM2);

        // Notify server that a character was received
        Send(t_id, message, 1, reply, 0);
    }
}

// Continously spins for sensor data from the tc server and sends it 
// to the terminal task. 
// Will eventually be moved to notify the task controlling the trains.
void sensor_state_notifier()
{
	int tcid = WhoIs("tc_server");
	int tid = WhoIs("terminal");
	int cid = WhoIs("clock_server");
	while(cid < 0) cid = WhoIs("clock_server");

	char sensor_dump[10];
	char reply[1];

	for(;;)
	{
		GetSensors(tcid, sensor_dump);

		// Notify server that a sensor dump was completed
        Send(tid, sensor_dump, 10, reply, 0);
	}
}

// TEMP
void add_sensor_to_queue(int sensor)
{
	for(int i = 15; i != 0; i--) sensor_states[i] = sensor_states[i-1];
	sensor_states[0] =  sensor;
}

// TEMP
// TODO Generalize to be used in TC1 and TC2
// Parses a 10 byte sensor dump and adds newly triggered sensors 
// to the sensor queue.
int parse_sensor_states(char* sensor_bytes)
{
	int updated = 0;

	for( int i = 0; i != 10; i+=2)
	{
		char byte1 = sensor_bytes[i];
		char byte2 = sensor_bytes[i+1];
		int data = byte1;
		data = data << 8;
		data += byte2;

		int sensor_bank = i/2;

		for(int i = 16; i != 0; i--)
		{
			int sensor = sensor_bank * 16 + i;
			if(data & 1) 
			{
				if(!all_sensor_states[sensor])
				{
					updated = 1;
					add_sensor_to_queue(sensor_bank * 16 + (i-1)); 
					all_sensor_states[sensor] = 1;
				}
			}
			else
			{
				all_sensor_states[sensor] = 0;
			}
			data = data >> 1;
		}
	}

	return updated;
}

// For printing the time and debugging spinner print
void terminal_tick_notifier()
{
    int tcid = WhoIs("terminal");
    int csid = WhoIs("clock_server");

    char message[4];
    char reply[1];

    for(;;)
    {
        int time = Delay(csid, TERMINAL_TICK_NOTIFIER_DELAY);
        pack_int(time, message);

        // Notify server that time has changed
        Send(tcid, message, 4, reply, 0);
    }
}

// Notify the terminal when track initialization is complete
void track_initialized_notifier()
{
	int tcid = WhoIs("tc_server");
	int tid = WhoIs("terminal");

	char msg[1];
	char reply[1];

	InitComplete(tcid);

	// Notify server that tc server initialization has completed
	Send(tid, msg, 0, reply, 0);
}

void terminal(void)
{
	unsigned int* track_a_straight[] = 
	{
	L"──────────────────── ╭─────────────────────────────────────────────────────────────────────────────╮ \n\r",
	L"                  12 │     ╭ 11                         13           10                            │ \n\r",
	L"───────────────── ╭──╯     │╭─────────────────────────────────────────────────────────────────────╮│ \n\r",
	L"                4 │        │ 14                         ╮             ╭                          8 │ \n\r",
	L"──────────────────╯        │                            │      │ 156  │                            │ \n\r",
	L"                           │                            │  155 │╭─────╯                            │ \n\r",
	L"                           │                            ╰─────╮│                                   │ \n\r",
	L"                           │                                   │                                   │ \n\r",
	L"                           │                                   │                                   │ \n\r",
	L"                           │                               153 │                                   │ \n\r",
	L"                           │                            ╭─────╯│ 154                               │ \n\r",
	L"                           │                            │      │╰─────╮                            │ \n\r",
	L"────────────────╮          │                            │      │      │                            │ \n\r",
	L"              1 │          │ 15                         ╯ 16       17 ╰                          9 │ \n\r",
	L"─────────────── ╰──╮       │╰─────────────────────────────────────────────────────────────────────╯│ \n\r",
	L"                 2 │       │                                                                       │ \n\r",
	L"────────────────── ╰──╮    ╰───────────────────────────────────────────────────────────────────────╯ \n\r",
	L"                      │                               6 ╮             ╭ 7                            \n\r",
	L"                    3 ╰                              18 ╰             ╯ 5                            \n\r",
	L"──────────────────────────────────────────────────────────────────────────────────────────────────── \n\r"
	};

	unsigned int* track_a_curved[] = 
	{
	L"────────────────────────── ╭───────────────────────────────────────────────────────────────────────╮ \n\r",
	L"                  12 ╭     │ 11                         13           10                            │ \n\r",
	L"─────────────────────╯     ╭────────────────────────────╮ ─────────── ╭────────────────────────────╮ \n\r",
	L"                4 ╭        │ 14                         │             │                          8 │ \n\r",
	L"──────────────────╯        │                            │      │ 156  │                            │ \n\r",
	L"                           │                            │  155 ╭──────╯                            │ \n\r",
	L"                           │                            ╰──────╮                                   │ \n\r",
	L"                           │                                   │                                   │ \n\r",
	L"                           │                                   │                                   │ \n\r",
	L"                           │                               153 │                                   │ \n\r",
	L"                           │                            ╭──────╯ 154                               │ \n\r",
	L"                           │                            │      ╰──────╮                            │ \n\r",
	L"────────────────╮          │                            │      │      │                            │ \n\r",
	L"              1 ╰          │ 15                         │ 16       17 │                          9 │ \n\r",
	L"───────────────────╮       ╰────────────────────────────╯ ─────────── ╰────────────────────────────╯ \n\r",
	L"                 2 ╰       │                                                                       │ \n\r",
	L"──────────────────────╮    ╰────────────────────────────╮ ─────────── ╭────────────────────────────╯ \n\r",
	L"                      │                               6 │             │ 7                            \n\r",
	L"                    3 │                              18 │             │ 5                            \n\r",
	L"───────────────────── ╰──────────────────────────────── ╰─────────────╯ ──────────────────────────── \n\r"
	};
	
	int track_a_turnout_types[SWITCH_COUNT] = 
	{
		TURNOUT_LEFT_UP,
		TURNOUT_LEFT_UP,
		TURNOUT_LEFT_UP,
		TURNOUT_DOWN_LEFT,
		TURNOUT_UP_RIGHT,
		TURNOUT_RIGHT_DOWN,
		TURNOUT_DOWN_LEFT,
		TURNOUT_LEFT_UP,
		TURNOUT_DOWN_LEFT,
		TURNOUT_DOWN_LEFT,
		TURNOUT_DOWN_LEFT,
		TURNOUT_DOWN_LEFT,
		TURNOUT_RIGHT_DOWN,
		TURNOUT_RIGHT,
		TURNOUT_RIGHT,
		TURNOUT_UP_RIGHT,
		TURNOUT_LEFT_UP,
		TURNOUT_LEFT_UP,

		TURNOUT_LEFT,
		TURNOUT_RIGHT,
		TURNOUT_LEFT,
		TURNOUT_RIGHT
	};

	int track_a_turnout_positions[SWITCH_COUNT*2] = 
	{
		14, 16,
		16, 19,
		19, 22,
		2, 18,
		19, 70,
		16, 56,
		16, 70,
		2, 99,
		14, 99,
		2, 70,
		0, 27,
		0, 21,
		2, 56,
		2, 27,
		14, 27,
		14, 56,
		14, 70,
		19, 56,

		10, 63,
		11, 63,
		6, 63,
		5, 63,
	};

	unsigned int* track_b_straight[] = 
	{
	L"──────────────────────────────────────────────────────────────────────────────────────────────────── \n\r",
    L"                           5 ╭             ╮ 18                               ╮ 3                    \n\r",
    L"                           7 ╯             ╰ 6                                │                      \n\r",
	L"╭───────────────────────────────────────────────────────────────────────╮     ╰──╮ ───────────────── \n\r",
	L"│                            17           16                            │        │ 2                 \n\r",
	L"│╭─────────────────────────────────────────────────────────────────────╮│        ╰──╮ ────────────── \n\r",
	L"│ 8                          ╮             ╭                         15 │           │ 1              \n\r",
	L"│                            │  154 │      │                            │           │                \n\r",
	L"│                            ╰─────╮│ 153  │                            │           │                \n\r",
	L"│                                   │╭─────╯                            │           │                \n\r",
	L"│                                   │                                   │           │                \n\r",
	L"│                                   │                                   │           │                \n\r",
	L"│                                   │ 156                               │           │                \n\r",
	L"│                               155 │╰─────╮                            │           │                \n\r",
	L"│                            ╭─────╯│      │                            │           │                \n\r",
	L"│                            │      │      │                            │        ╭──╯                \n\r",
	L"│ 9                          ╯ 10       13 ╰                         14 │      4 │                   \n\r",
	L"│╰─────────────────────────────────────────────────────────────────────╯│     ╭──╯ ───────────────── \n\r",
	L"│                                                                    11 ╯  12 ╯                      \n\r",
	L"╰─────────────────────────────────────────────────────────────────────────────────────────────────── \n\r",
	};

	unsigned int* track_b_curved[] = 
	{
	L"──────────────────────────── ╭─────────────╮ ─────────────────────────────────╮ ──────────────────── \n\r",
    L"                           5 │             │ 18                               │ 3                    \n\r",
    L"                             │             │ 6                                │                      \n\r",
	L"╭────────────────────────────╯ ─────────── ╰────────────────────────────╮     ╰───────────────────── \n\r",
	L"│                            17           16                            │        ╮ 2                 \n\r",
	L"╭────────────────────────────╮ ─────────── ╭────────────────────────────╮        ╰────────────────── \n\r",
	L"│ 8                          │             │                         15 │           ╮ 1              \n\r",
	L"│                            │  154 │      │                            │           │                \n\r",
	L"│                            ╰──────╮ 153  │                            │           │                \n\r",
	L"│                                   ╭──────╯                            │           │                \n\r",
	L"│                                   │                                   │           │                \n\r",
	L"│                                   │                                   │           │                \n\r",
	L"│                                   │ 156                               │           │                \n\r",
	L"│                               155 ╰──────╮                            │           │                \n\r",
	L"│                            ╭──────╯      │                            │           │                \n\r",
	L"│                            │      │      │                            │        ╭──╯                \n\r",
	L"│ 9                          │ 10       13 │                         14 │      4 ╯                   \n\r",
	L"╰────────────────────────────╯ ─────────── ╰────────────────────────────╯     ╭───────────────────── \n\r",
	L"│                                                                    11 │  12 │                      \n\r",
	L"╰───────────────────────────────────────────────────────────────────────╯ ────╯ ──────────────────── \n\r",
	};

	int track_b_turnout_types[SWITCH_COUNT] = 
	{
		TURNOUT_RIGHT_DOWN,
		TURNOUT_RIGHT_DOWN,
		TURNOUT_RIGHT_DOWN,
		TURNOUT_UP_RIGHT,
		TURNOUT_DOWN_LEFT,
		TURNOUT_LEFT_UP,
		TURNOUT_UP_RIGHT,
		TURNOUT_RIGHT,
		TURNOUT_RIGHT,
		TURNOUT_UP_RIGHT,
		TURNOUT_UP_RIGHT,
		TURNOUT_UP_RIGHT,
		TURNOUT_LEFT_UP,
		TURNOUT_LEFT,
		TURNOUT_LEFT,
		TURNOUT_DOWN_LEFT,
		TURNOUT_RIGHT_DOWN,
		TURNOUT_RIGHT_DOWN,

		TURNOUT_RIGHT,
		TURNOUT_LEFT,
		TURNOUT_LEFT,
		TURNOUT_RIGHT
	};

	int track_b_turnout_positions[SWITCH_COUNT*2] = 
	{
		5, 84,
		3, 81,
		0, 78,
		17,81,
		0, 29,
		3, 43,
		3, 29,
		5, 0,
		17, 0,
		17,29,
		19,72,
		19,78,
		17,43,
	    17,72,
		5, 72,
		5, 43,
		5, 29,
		0, 43,

		9, 36,
		8, 36,
		14,36,
		13,36
	};

	pid = WhoIs("com2");
	com1_id = WhoIs("com1");
	int csid = WhoIs("clock_server");

	RegisterAs("terminal");
	int sender;

	// Initial output setup
	ClearScreen(pid);
    MoveCursor(pid, 0, 3);
	Print(pid, "\033[?25l"); // Hide Cursor

	// Initialize variables for displaying
	for(int i = 0; i != 16; i++) sensor_states[i] = -1;
	for(int i = 0; i != 128; i++) all_sensor_states[i] = 0;

	int input_notifier_id = Create(TERMINAL_NOTIFIER_PRIORITY, input_notifier);
	int sensor_state_notifier_id = -1; // Created after initialization
	int terminal_tick_notifer_id = Create(TERMINAL_NOTIFIER_PRIORITY, terminal_tick_notifier);
	int track_initialized_notifier_id = Create(TERMINAL_NOTIFIER_PRIORITY, track_initialized_notifier);

	// Debugging spinner variables
	// TODO Move spinner to a new task entirely? Since timer currently accomplishes same task.
	char *spinner = "-\\|/-\\|/"; 
	int spinner_state = 0;
	int spinner_length = _strlen(spinner);
	int spinner_ticks_per_frame = 10;
	spinner_ticks_per_frame /= TERMINAL_TICK_NOTIFIER_DELAY;

	int time = Time(csid);

	track_initialized = 0;

	// Initialize switch states for printing
	// TODO Rememdy code repetition between tc_server and here?
	int switches[SWITCH_COUNT];
	for(int i = 0; i != 18; i++) switches[i] = i + 1;
	switches[18] = 0x99;
	switches[19] = 0x9A;
	switches[20] = 0x9B;
	switches[21] = 0x9C;
	for(int i = 0; i != 256; i++) switch_states[i] = 0;
	for(int i = 0; i != SWITCH_COUNT; i++) switch_states[switches[i]] = 'S';
	switch_updated = 1;

	int switch_to_index[0x9C + 1];
	for(int i = 1; i != 19; i++) switch_to_index[i] = i - 1;
	switch_to_index[0x99] = 18;
	switch_to_index[0x9A] = 19;
	switch_to_index[0x9B] = 20;
	switch_to_index[0x9C] = 21;

	// Initialize command buffer
    command_len = 0;
    last_command_len = 0;
    command[0] = 0;
	char msg[MAX_TPRINT_SIZE];

	MoveCursor(pid, 3, INITIALIZATION_PRINT_HEIGHT);
	Print(pid, "INITIALIZING...");

	active_track = TRACK_A;

	print_track(pid, track_a_straight, TRACK_PRINT_COL, TRACK_PRINT_HEIGHT);

	init_track_view( &track_a_straight_view, track_a_straight, TRACK_DISPLAY_WIDTH, TRACK_DISPLAY_HEIGHT, TRACK_PRINT_COL, TRACK_PRINT_HEIGHT,
					 switch_to_index, track_a_turnout_types, track_a_turnout_positions); 
	init_track_view( &track_a_curved_view, track_a_curved, TRACK_DISPLAY_WIDTH, TRACK_DISPLAY_HEIGHT, TRACK_PRINT_COL, TRACK_PRINT_HEIGHT,
					 switch_to_index, track_a_turnout_types, track_a_turnout_positions); 
	init_track_view( &track_b_straight_view, track_b_straight, TRACK_DISPLAY_WIDTH, TRACK_DISPLAY_HEIGHT, TRACK_PRINT_COL, TRACK_PRINT_HEIGHT,
					 switch_to_index, track_b_turnout_types, track_b_turnout_positions); 
	init_track_view( &track_b_curved_view, track_b_curved, TRACK_DISPLAY_WIDTH, TRACK_DISPLAY_HEIGHT, TRACK_PRINT_COL, TRACK_PRINT_HEIGHT,
					 switch_to_index, track_b_turnout_types, track_b_turnout_positions); 

    for(;;)
    {
		Receive(&sender, msg, MAX_TPRINT_SIZE);

		// Message received was a new char from the user
		if(sender == input_notifier_id)
		{
			// Save where cursor was
			Print(pid, "\033[s");

			int c = msg[0];

			// If Getc errored, try again
			// TODO Perhaps yield?
			if(c < 0) continue;

			switch(c)
			{
				case 8: // Backspace
					command_popback();
					break;

				case 27: // ESC
					if(!track_initialized) break;
					Shutdown();
					break;

				case 13: // Return pressed
					clear_invalid_message();
					process_command();
					command_clear();
					break;

				default:
					if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == ' ' || (c >= '0' && c <= '9')) 
						command_append(c);
					break;
			}

			print_command();
			last_command_len = command_len;

			// Load saved cursor
			Print(pid, "\033[u");
		}
		else if(sender == sensor_state_notifier_id)
		{
			// A sensor dump has occured

			MoveCursor(pid, 0,20);

			// If sensor states have been updated, reprint the sensor state queue
			if(parse_sensor_states(msg))
			{
				MoveCursor(pid, 1, SENSOR_PRINT_HEIGHT);
				Print(pid, "LAST 16 SENSOR HITS: ");
				MoveCursor(pid, 22, SENSOR_PRINT_HEIGHT);
				char sensor[32];

				for(int i = 0; i != 16; i++)
				{
					if(sensor_states[i] == -1) break;
					itos(sensor_states[i], sensor);

					char sensor_str[4];
					sensor_str[3] = 0;
					char* number_part =sensor_str + 1;

					sensor_str[0] = 'A' + sensor_states[i]/16;
					itos(sensor_states[i]%16+1, number_part);

					Print(pid, sensor_str);
					Putc(pid, COM2, ' ');
				}
			}

		}
		else if (sender == terminal_tick_notifer_id)
		{
			// Update time and spinner

			time = unpack_int(msg);

			// Save where cursor was
			Print(pid, "\033[s");

			MoveCursor(pid, 1, TIME_PRINT_HEIGHT);
			print_time(time);

			// Load saved cursor
			Print(pid, "\033[u");

			if(time % spinner_ticks_per_frame == 0)
			{
				// Save where cursor was
				Print(pid, "\033[s");

				MoveCursor(pid, 0, SPINNER_PRINT_HEIGHT);
				Putc(pid, COM2, spinner[spinner_state]);

				// Load saved cursor
				Print(pid, "\033[u");
				spinner_state++;
				spinner_state %= spinner_length;
			}
		}
		else if (sender == track_initialized_notifier_id)
		{
			sensor_state_notifier_id = Create(TERMINAL_NOTIFIER_PRIORITY, sensor_state_notifier);
			track_initialized = 1;
			MoveCursor(pid, 3, INITIALIZATION_PRINT_HEIGHT);
			Print(pid, "                ");
		}
		else // Task requested to print to terminal
		{
			// For now, just print it.
			// TODO Move prints from other tasks to a designated scrolling area.
			UPrint(pid, msg);
		}

		// Reprint switch states if they have changed
		if(switch_updated)
		{
			MoveCursor(pid, 1, SWITCH_PRINT_HEIGHT);
			Print(pid, "SWITCHES:");

			MoveCursor(pid, 1, SWITCH_PRINT_HEIGHT + 1);
			char switch_number[32];

			int spacing = 5;

			for(int i = 0; i != SWITCH_COUNT; i++)
			{
				format_string ( switch_number, 32, "%d", switches[i] );

				int space_left = spacing - _strlen(switch_number);

				Print(pid, switch_number);
				for (int i = 0; i != space_left; i++) Putc(pid, COM2, ' ');
			}

			MoveCursor(pid, 1, SWITCH_PRINT_HEIGHT + 2);
			for(int i = 0; i != SWITCH_COUNT; i++)
			{
				char switch_state = switch_states[switches[i]];

				Putc(pid, COM2, switch_state);
				for (int i = 0; i != spacing - 1; i++) Putc(pid, COM2, ' ');
			}

			switch_updated = 0;
		}
		
		Reply(sender, msg, 0);
    }
}
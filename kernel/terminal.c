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
#include "train_control_server.h"
#include "sensors.h"

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
#define MAX_READ_STRING_SIZE 16

#define TERMINAL_TICK_NOTIFIER_DELAY 5
#define TERMINAL_NOTIFIER_PRIORITY 4

#define TPRINT_COMMAND 1
#define TPRINTAT_COMMAND 2
#define PRINT_TRACK_COMMAND 3

// The following variables are used by functions only called by the terminal task.
// Although "global", these variables are accessed by no other task
// than the terminal task and thus do not violate memory exclusivity.
// These exist to avoid constantly passing large amounts
// of variables to functions only called by the terminal task.
static char command[MAX_COMMAND_LEN + 1];
static int command_len;
static int last_command_len;

static int pid;
static int com1_id;
static int tcid;
static int train_control_server_id;

static int track_initialized;

// TEMP 
// Variables used to hold senor states, and a queue of the last 16
// triggered sensors. Declared volatile to avoid the compiler using undefined 
// functions to optimize operations done on them.
static volatile int sensor_states[16];
static volatile int all_sensor_states[128];

// Variables used to store switches triggered and changed. 
// In the future this responsibility may move the tc_server.
// static int switches[SWITCH_COUNT];
static char switch_states[256];
static int switch_updated;

static TrackView track_a_curved_view;
static TrackView track_a_straight_view;
static TrackView track_b_curved_view;
static TrackView track_b_straight_view;
static char active_track;

void TPrint(int tid, char* str, ... )
{
	va_list va;
	va_start(va, str);

    char message[MAX_TPRINT_SIZE + 1];
    char reply[1];

	message[0] = TPRINT_COMMAND;

	// If formatted string is longer than MAX_TPRINT_SIZE,
	// it should be truncated.
	_format_string(message + 1, MAX_TPRINT_SIZE, str, va);

    Send(tid, message, MAX_TPRINT_SIZE + 1, reply, 0);

	va_end(va);
}

void PrintTrack(int tid, char track)
{
	char message[2];
    char reply[1];

	message[0] = PRINT_TRACK_COMMAND;
	message[1] = track;

	Send(tid, message, 2, reply, 0);
}

void TPrintAt(int tid, int x, int y, char* str, ...)
{
	va_list va;
	va_start(va, str);

    char message[MAX_TPRINT_SIZE + 1];
    char reply[1];

	message[0] = TPRINTAT_COMMAND;

	// If formatted string is longer than MAX_TPRINT_SIZE,
	// it should be truncated.
	int new_start = format_string(message + 1, MAX_TPRINT_SIZE, "\033[%d;%dH", y, x);
	_format_string(message + 1 + new_start, MAX_TPRINT_SIZE - new_start, str, va);
	
    Send(tid, message, MAX_TPRINT_SIZE + 1, reply, 0);

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

	TPrintAt(pid, 0, COMMAND_PRINT_HEIGHT, cleared);
}

void print_command()
{
	if(command_len == last_command_len) return;
	else if(command_len > last_command_len)
	{
		// Make cursor go to bottom left.
		// MoveCursor(pid, command_len, COMMAND_PRINT_HEIGHT);
		TPrintAt(pid, command_len, COMMAND_PRINT_HEIGHT, "%c", command[command_len-1]);
	}
	else
	{
		// Make cursor go to bottom left.
		// MoveCursor(pid, 1+command_len, COMMAND_PRINT_HEIGHT);
		TPrintAt(pid, 1+command_len, COMMAND_PRINT_HEIGHT, " ");
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

// Increment *command past string if another word exists.
// Returns if the command contained another word
int read_string(char* buffer, char** command)
{
	// Skip whitespace
	while(**command == ' ') (*command)++;

	// If end of string, return no new word
	if(**command == 0) return 0;

	int read = 0;
	while(**command != 0 && **command != ' ' && read < MAX_READ_STRING_SIZE)
	{
		*buffer = **command;
		read++;
		buffer++;
		(*command)++;
	}

	*buffer = 0;

	return 1;
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

void print_invalid_path()
{
	TPrintAt(pid, 0, COMMAND_PRINT_HEIGHT - 1, "Path from source to destination does not exist!\n\r");
}

void print_invalid_switch_track()
{
	TPrintAt(pid, 0, COMMAND_PRINT_HEIGHT - 1, "Ensure all turnouts are set to straight before switching tracks!\n\r");
}

void print_initialization_incomplete()
{
	TPrintAt(pid, 0, COMMAND_PRINT_HEIGHT - 1, "Initialization incomplete, command ignored!\n\r");
}

void print_invalid_argument()
{
	TPrintAt(pid, 0, COMMAND_PRINT_HEIGHT - 1, "Invalid argument provided!\n\r");
}

void print_invalid_command()
{
	TPrintAt(pid, 0, COMMAND_PRINT_HEIGHT - 1, "Invalid command provided!\n\r");
}

void clear_invalid_message()
{
	TPrintAt(pid, 0, COMMAND_PRINT_HEIGHT - 1, "                                                                            \n\r");
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

	TPrintAt(pid, 1, TIME_PRINT_HEIGHT,"%s:%s.%s\n\r\n\r", minutes_str, seconds_str, tenths_str);
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
			PrintTrack(pid, 'A');
			SetTrack(train_control_server_id, 'A');
		} 
		else if(is_arg("B", &command_p))
		{
			active_track = TRACK_B;
			PrintTrack(pid, 'B');
			SetTrack(train_control_server_id, 'B');
		} 
		else print_invalid_argument();
	}
	else if (is_command("it", &command_p)) 
	{
		int t_number = parse_int(&command_p);
		// int t_node = parse_int(&command_p);
		// int t_target_node = parse_int(&command_p);

		char t_node_string[MAX_READ_STRING_SIZE];
		char t_target_node_string[MAX_READ_STRING_SIZE];

		read_string(t_node_string, &command_p);
		read_string(t_target_node_string, &command_p);

		// print(" %s ", t_node_string);
		// print(" %s ", t_target_node_string);
		// assert(0);

		if(_strlen(t_node_string) > 3 || _strlen(t_node_string) < 2 ||
		   _strlen(t_target_node_string) > 3 || _strlen(t_target_node_string) < 2) 
		   {
			   print_invalid_argument();
			   return;
		   }

		int t_node = sensor_string_index(t_node_string);
		int t_target_node = sensor_string_index(t_target_node_string);

		if(t_node < 0 || t_target_node < 0) 
		{
			print_invalid_argument();
			return;
		}
			
		int t_target_speed = parse_int(&command_p);

		if(command_p == 0) print_invalid_argument();
		if(!is_valid_speed(t_target_speed))
		{
			print_invalid_argument();
			return;
		} 

		if(InitTrain(train_control_server_id, t_number, t_node, t_target_node, t_target_speed) == NO_PATH_EXISTS)
			print_invalid_path();
	}
	else if (is_command("tp", &command_p)) 
	{
		int t_number = parse_int(&command_p);
		int t_target_node = parse_int(&command_p);
		int t_offset = parse_int(&command_p);

		if(command_p == 0) print_invalid_argument();

		if(TargetPosition(train_control_server_id, t_number, t_target_node, t_offset))
			print_invalid_path();
	}
	else if (is_command("sl", &command_p)) 
	{
		// Inner loop is the same regardless of track!
		SwitchTrackAsync(tcid, 10, STRAIGHT);
		SwitchTrackAsync(tcid, 13, STRAIGHT);
		SwitchTrackAsync(tcid, 16, STRAIGHT);
		SwitchTrackAsync(tcid, 17, STRAIGHT);
		SwitchTrackAsync(tcid, 8,  CURVED);
		SwitchTrackAsync(tcid, 9,  CURVED);
		SwitchTrackAsync(tcid, 14, CURVED);
		SwitchTrackAsync(tcid, 15, CURVED);
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

void terminal_printer()
{
	RegisterAs("terminal_output");
	int pid = WhoIs("com2");

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

	int switch_to_index[0x9C + 1];
	for(int i = 1; i != 19; i++) switch_to_index[i] = i - 1;
	switch_to_index[0x99] = 18;
	switch_to_index[0x9A] = 19;
	switch_to_index[0x9B] = 20;
	switch_to_index[0x9C] = 21;

	init_track_view( &track_a_straight_view, track_a_straight, TRACK_DISPLAY_WIDTH, TRACK_DISPLAY_HEIGHT, TRACK_PRINT_COL, TRACK_PRINT_HEIGHT,
					 switch_to_index, track_a_turnout_types, track_a_turnout_positions); 
	init_track_view( &track_a_curved_view, track_a_curved, TRACK_DISPLAY_WIDTH, TRACK_DISPLAY_HEIGHT, TRACK_PRINT_COL, TRACK_PRINT_HEIGHT,
					 switch_to_index, track_a_turnout_types, track_a_turnout_positions); 
	init_track_view( &track_b_straight_view, track_b_straight, TRACK_DISPLAY_WIDTH, TRACK_DISPLAY_HEIGHT, TRACK_PRINT_COL, TRACK_PRINT_HEIGHT,
					 switch_to_index, track_b_turnout_types, track_b_turnout_positions); 
	init_track_view( &track_b_curved_view, track_b_curved, TRACK_DISPLAY_WIDTH, TRACK_DISPLAY_HEIGHT, TRACK_PRINT_COL, TRACK_PRINT_HEIGHT,
					 switch_to_index, track_b_turnout_types, track_b_turnout_positions); 

	char msg[MAX_TPRINT_SIZE + 1];
	char reply[1];
	int msg_size;
	int sender;

	for(;;)
	{
		msg_size = Receive(&sender, msg, MAX_TPRINT_SIZE + 1);

		switch(msg[0])
		{
			case TPRINT_COMMAND:
				
				// For sanity, throw a null terminator at the end of the message
				if(msg_size > MAX_TPRINT_SIZE + 1)  msg[MAX_TPRINT_SIZE] = 0;
				else msg[msg_size] = 0;

				// For now, just print it.
				// TODO Move TPrints from other tasks to a designated scrolling area.
				UPrint(pid, msg + 1);

				break;
				
			case TPRINTAT_COMMAND:
				// For sanity, throw a null terminator at the end of the message
				if(msg_size > MAX_TPRINT_SIZE + 1)  msg[MAX_TPRINT_SIZE] = 0;
				else msg[msg_size] = 0;
				// Print the message, has the location encoded in it already.
				UPrint(pid, msg + 1);

				break;

			case PRINT_TRACK_COMMAND:
				if(msg[1] == 'A')
					print_track(pid, track_a_straight_view.data, TRACK_PRINT_COL, TRACK_PRINT_HEIGHT);
				else if(msg[1] == 'B')
					print_track(pid, track_b_straight_view.data, TRACK_PRINT_COL, TRACK_PRINT_HEIGHT);

				break;
				
			default:
				// Unrecognized command sent!
				assert(0);
				break;
		}

		Reply(sender, reply, 0);
	}
}

void terminal(void)
{
	RegisterAs("terminal");
	pid = Create(2, terminal_printer); // NOTE was 4
	com1_id = WhoIs("com1");
	tcid = WhoIs("tc_server");
	train_control_server_id = WhoIs("train_control");

	int csid = WhoIs("clock_server");

	int sender;

	// Initial output setup
	ClearScreen(pid);
	TPrintAt(pid, 0, 3, "\033[?25l"); // Hide Cursor

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

	// int switch_to_index[0x9C + 1];
	// for(int i = 1; i != 19; i++) switch_to_index[i] = i - 1;
	// switch_to_index[0x99] = 18;
	// switch_to_index[0x9A] = 19;
	// switch_to_index[0x9B] = 20;
	// switch_to_index[0x9C] = 21;

	// Initialize command buffer
    command_len = 0;
    last_command_len = 0;
    command[0] = 0;
	char msg[MAX_TPRINT_SIZE + 1];

	TPrintAt(pid, 3, INITIALIZATION_PRINT_HEIGHT, "INITIALIZING...");

	active_track = TRACK_A;
	PrintTrack(pid, 'A');

    for(;;)
    {
		Receive(&sender, msg, MAX_TPRINT_SIZE + 1);

		// Message received was a new char from the user
		if(sender == input_notifier_id)
		{
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
					if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == ' ' || (c >= '0' && c <= '9') || c == '-') 
						command_append(c);
					break;
			}

			print_command();
			last_command_len = command_len;
		}
		else if(sender == sensor_state_notifier_id)
		{
			// A sensor dump has occured

			MoveCursor(pid, 0,20);

			// If sensor states have been updated, reprint the sensor state queue
			if(parse_sensor_states(msg))
			{
				TPrintAt(pid, 1, SENSOR_PRINT_HEIGHT, "LAST 16 SENSOR HITS: ");

				int x = 22;

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

					TPrintAt(pid, x, SENSOR_PRINT_HEIGHT, sensor_str);
					x += _strlen(sensor_str);
					TPrintAt(pid, x, SENSOR_PRINT_HEIGHT, " ");
					x += 1;
				}
			}

		}
		else if (sender == terminal_tick_notifer_id)
		{
			// Update time and spinner

			time = unpack_int(msg);

			print_time(time);

			if(time % spinner_ticks_per_frame == 0)
			{
				TPrintAt(pid, 0, SPINNER_PRINT_HEIGHT, "%c", spinner[spinner_state]);

				spinner_state++;
				spinner_state %= spinner_length;
			}
		}
		else if (sender == track_initialized_notifier_id)
		{
			sensor_state_notifier_id = Create(TERMINAL_NOTIFIER_PRIORITY, sensor_state_notifier);
			track_initialized = 1;
			TPrintAt(pid, 3, INITIALIZATION_PRINT_HEIGHT, "                ");
		}
		else // Task requested to print to terminal
		{
			assert(0);
		}

		// Reprint switch states if they have changed
		if(switch_updated)
		{
			TPrintAt(pid, 1, SWITCH_PRINT_HEIGHT, "SWITCHES:");
			int x = 1;

			char switch_number[32];

			int spacing = 5;

			for(int i = 0; i != SWITCH_COUNT; i++)
			{
				format_string ( switch_number, 32, "%d", switches[i] );

				int space_left = spacing - _strlen(switch_number);

				TPrintAt(pid, x, SWITCH_PRINT_HEIGHT + 1, switch_number);
				x += 4;

				for (int i = 0; i != space_left; i++) Putc(pid, COM2, ' ');
			}

			// MoveCursor(pid, 1, SWITCH_PRINT_HEIGHT + 2);
			x = 1;
			for(int i = 0; i != SWITCH_COUNT; i++)
			{
				char switch_state = switch_states[switches[i]];

				TPrintAt(pid, x, SWITCH_PRINT_HEIGHT + 2, "%c", switch_state);
				x+=4;

				for (int i = 0; i != spacing - 1; i++) Putc(pid, COM2, ' ');
			}

			switch_updated = 0;
		}

		// MoveCursor(pid, 0, 48);
		// Print(pid, "tc switch buffer end = %d             \n\r", com1_queue_end);
		// MoveCursor(pid, 0, 49);
		// Print(pid, "tc switch buffer start = %d             \n\r", com1_queue_start);
		// MoveCursor(pid, 0, 50);
		// Print(pid, "tc switch buffer size = %d             \n\r", com1_queue_size);
		
		Reply(sender, msg, 0);
    }
}
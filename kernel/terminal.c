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

#define COMMAND_PRINT_HEIGHT 24
#define MAX_COMMAND_LEN 127
#define MAX_TPRINT_SIZE 1024

char command[MAX_COMMAND_LEN + 1];
int command_len;
int last_command_len;

int pid;
int com1_id;
int tcid;

// TEMP 
volatile int sensor_states[16];
volatile int all_sensor_states[128];

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

// Increment *command if the command matches.
int is_command(char* prefix, char** command)
{
	int prefix_len = _strlen(prefix);
	int r = _strsim(prefix, *command) == prefix_len;
	if(r) (*command) += prefix_len;
	return r;
}

// Increment *command if the argument matches.
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
	Print(pid, "                                \n\r");
}

// TEMP Pulled from a0 and should be replaced.
void process_command()
{	
	if (command_len == 0) return;
	// if (command_len == 1 && command[0] == 'q'); // Do nothing currently

	char* command_p = command;

	if (command_len == 1) return;
	if (is_command("tr", &command_p)) 
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

		if(is_arg("C", &command_p)) SwitchTrack(tcid, s_number, CURVED);
		else if(is_arg("S", &command_p)) SwitchTrack(tcid, s_number, STRAIGHT);
		else print_invalid_argument();
	}
	else
	{
		print_invalid_command();
	}
}

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

void sensor_state_notifier()
{
	tcid = WhoIs("tc_server");
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

// For a debugging spinner print
void terminal_tick_notifier()
{
    int tcid = WhoIs("terminal");
    int csid = WhoIs("clock_server");

    char message[4];
    char reply[1];

    for(;;)
    {
        int time = Delay(csid, 25);
        pack_int(time, message + 1);

        // Notify server that time has changed
        Send(tcid, message, 4, reply, 0);
    }
}

void terminal(void)
{
	pid = WhoIs("com2");
	com1_id = WhoIs("com1");
	// int csid = WhoIs("clock_server");

	RegisterAs("terminal");
	int sender;

	// Initial output setup
	ClearScreen(pid);
    MoveCursor(pid, 0, 3);
	Print(pid, "\033[?25l"); // Hide Cursor

	// Initialize variables for displaying
	for(int i = 0; i != 16; i++) sensor_states[i] = -1;
	for(int i = 0; i != 128; i++) all_sensor_states[i] = 0;

	int input_notifier_id = Create(3, input_notifier);
	int sensor_state_notifier_id = Create(3, sensor_state_notifier);
	int terminal_tick_notifer_id = Create(3, terminal_tick_notifier);

	char spinner[] = {'-', '\\', '|', '/', '-','\\', '|', '/'};
	int spinner_state = 0;

    command_len = 0;
    last_command_len = 0;
    command[0] = 0;
	char msg[MAX_TPRINT_SIZE];

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
					// Do nothing currently
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
			MoveCursor(pid, 0,20);

			// If sensor states have been updated, reprint the sensor state queue
			if(parse_sensor_states(msg))
			{
				MoveCursor(pid, 1, 8);
				Print(pid, "LAST 16 SENSOR HITS: ");
				MoveCursor(pid, 22, 8);
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
			// Save where cursor was
			Print(pid, "\033[s");

			MoveCursor(pid, 0, 3);
			Putc(pid, COM2, spinner[spinner_state]);

			// Load saved cursor
			Print(pid, "\033[u");
			spinner_state++;
			spinner_state %= 8;
		}
		else // Task requested to print to terminal
		{
			// For now, just print it.
			// TODO Violates servers not sending???
			UPrint(pid, msg);
		}
		
		Reply(sender, msg, 0);
    }
}
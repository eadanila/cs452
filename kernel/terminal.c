#include "terminal.h"
#include "name_server.h"
#include "uart_server.h"
#include <bwio.h>
#include "print.h"
#include "string_utility.h"
#include "syscall.h"
#include "logging.h"
#include "clock_server.h"

#define COMMAND_PRINT_HEIGHT 24
#define MAX_COMMAND_LEN 127
#define MAX_TPRINT_SIZE 1024

char command[MAX_COMMAND_LEN + 1];
int command_len;
int last_command_len;

int pid;
int com1_id;

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

        Putc(com1_id, COM1, t_speed);
        Putc(com1_id, COM1, t_number);
	}
	else if (is_command("rv", &command_p)) 
	{
		int t_number = parse_int(&command_p);
		if(command_p == 0) print_invalid_argument();
	}
	else if (is_command("sw", &command_p)) 
	{
		/*int s_number = */parse_int(&command_p);

		if(command_p == 0) print_invalid_argument();

		if(is_arg("C", &command_p)) {}
		else if(is_arg("S", &command_p)) {}
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

void terminal(void)
{
	pid = WhoIs("com2");
	com1_id = WhoIs("com1");

	RegisterAs("terminal");
	int sender;

	int input_notifier_id = Create(3, input_notifier);

    command_len = 0;
    last_command_len = 0;
    command[0] = 0;
	char msg[MAX_TPRINT_SIZE];

	// Initial output setup
	ClearScreen(pid);
    MoveCursor(pid, 0, 3);
	Print(pid, "\033[?25l"); // Hide Cursor

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
		else // Task requested to print to terminal
		{
			// For now, just print it.
			// TODO Violates servers not sending???
			UPrint(pid, msg);
		}
		
		Reply(sender, msg, 0);
    }
}
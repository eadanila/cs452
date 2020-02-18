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

// TEMP Pulled from a0 and should be replaced.
void process_command()
{	
	if (command_len == 0) return;
	// if (command_len == 1 && command[0] == 'q'); // Do nothing currently

	char* command_p = command;

	if (command_len == 1) return;
	if (_strsim("tr ", command) == 3) 
	{
		// MoveCursor(pid, 0, 10);
		// UPrint(pid, "COMMAND SENT");

		// TODO Error Check?
		command_p += 3;
		while(*command_p && *command_p == ' ') command_p++;
		int t_number = stoi(command_p);
		while(*command_p && *command_p != ' ') command_p++;
		while(*command_p && *command_p == ' ') command_p++;
		int t_speed = stoi(command_p);

        Putc(com1_id, COM1, t_speed);
        Putc(com1_id, COM1, t_number);
	}
	if (_strsim("rv ", command) == 3) 
	{
	}
	if (_strsim("sw ", command) == 3) 
	{
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
    MoveCursor(pid, 0, 0);
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
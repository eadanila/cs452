#include "constants.h"
#include "syscall.h"
#include "name_server.h"
#include "logging.h"
#include "task.h"

#include "string_utility.h"
// #include "string.h" // Doesn't work, get that str functions are undefined

// Prefixes for messages sent to name server to define the operation done on the name
#define REGISTERAS_MESSAGE_PREFIX 1
#define WHOIS_MESSAGE_PREFIX 2

char names[MAX_TASKS_ALLOWED][MAX_NAME_LENGTH];

int RegisterAs(const char *name)
{
    if(!is_valid_task(name_server_id)) 
        return INVALID_NAME_SERVER_TASK_ID;

    char message[MAX_NAME_LENGTH + 1];
    message[0] = REGISTERAS_MESSAGE_PREFIX;
    int msglen = _strlen(name) + 2;
    _strcpy(message + 1, name);
    char reply[1];
    
    Send(name_server_id, message, msglen, reply, 1);

    // Must case return to (signed char) since compiler 
    // implements char as unsigned char.
    return (signed char)reply[0];
}

int WhoIs(const char *name)
{
    if(!is_valid_task(name_server_id)) 
        return INVALID_NAME_SERVER_TASK_ID;

    char message[MAX_NAME_LENGTH + 1];
    message[0] = WHOIS_MESSAGE_PREFIX;
    int msglen = _strlen(name) + 2;
    _strcpy(message + 1, name);
    char reply[1];
    
    Send(name_server_id, message, msglen, reply, 1);

    return (signed char)reply[0];
}

int _who_is(char *name)
{
    // Search through all registered names to find a match
    for(int i = 0; i != MAX_TASKS_ALLOWED; i++)
    {
        if(names[i][0] && (_strcmp(name, names[i]) == 0)) return i;
    }

    return TASK_DNE;
}

int _register_as(int id, char *name)
{
    int existing = _who_is(name);
    // If name already exists, make that tasks name empty to guarantee
    // all names are unique
    if(existing != TASK_DNE) names[existing][0] = 0;
    
    _strcpy( names[id], name );
    return 0;
}

void print_name_server()
{
    print("==== name_server ====\n\r")
    for(int i = 0; i != MAX_TASKS_ALLOWED; i++)
    {
        if(names[i][0]) print("id: %d, name: %s \n\r", i, names[i]);
    }
    print("=====================\n\r")
}

void name_server(void)
{
    // Set all names to empty
    for( int i = 0; i != MAX_TASKS_ALLOWED; i++) names[i][0] = 0;

    // Currently the task id is returned by the name_server in a single
    // signed char.  Currently this allows for id's 0-127 and negative return
    // values. If MAX_TASKS_ALLOWED is to be increased past 128, this will 
    // need to be changed.
    assert(MAX_TASKS_ALLOWED <= 128);

    int sender;
    char msg[MAX_NAME_LENGTH + 1];
    char response[1];

    for(;;)
    {
        // Receive name server request
        Receive(&sender, msg, MAX_NAME_LENGTH);
        
        // Process request
        if(msg[0] == WHOIS_MESSAGE_PREFIX)
        {
            response[0] = _who_is(msg + 1);
        }
        else if (msg[0] == REGISTERAS_MESSAGE_PREFIX)
        {
            response[0] = _register_as(sender, msg + 1);
            // print_name_server(); // For debug
        }

        // Return result
        Reply(sender, response, 1);
    }
}
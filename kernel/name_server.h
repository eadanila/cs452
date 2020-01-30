#define INVALID_NAME_SERVER_TASK_ID -1
#define TASK_DNE -2
#define NAME_TOO_LONG -3

#define MAX_NAME_LENGTH 128 // 16 KiB total for all 128 names, includes null terminator 

/*
The task id of the name server must be known to the RegisterAs() 
and WhoIs() routines, which are executed by other tasks. This 
might be a minor violation of the memory separation principle, 
but is acceptable.
*/
int name_server_id; // Stores the task id of the name server for use by RegisterAs() and WhoIs()

int RegisterAs(const char *name);
int WhoIs(const char *name);

void name_server(void);
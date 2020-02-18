#ifndef TC_SERVER_H
#define TC_SERVER_H

#define STRAIGHT 'S'
#define CURVED 'C'

#define INVALID_TC_SERVER -1
#define INVALID_ARGUMENT -2

// Blocks until command is sent to the uart server
int SwitchTrack(int tid, char id, char dir);

// Returns immediatly, overwrites previous command if sent too quickly
int SetSpeed(int tid, char id, char speed);

// Returns immediatly, overwrites previous command if sent too quickly
int Reverse(int tid, char id);

void tc_server(void);

#endif
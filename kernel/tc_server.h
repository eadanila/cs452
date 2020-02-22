#ifndef TC_SERVER_H
#define TC_SERVER_H

#define STRAIGHT 33
#define CURVED 34

#define INVALID_TC_SERVER -1
#define INVALID_ARGUMENT -2

// NOTE: This is an initial interface meant to satisfy the requirements 
//       for a0 and may change during TC1 and TC2.

// Blocks until command is sent to the uart server
int SwitchTrack(int tid, char id, char dir);

// Returns immediatly, overwrites previous command if sent too quickly
int SetSpeed(int tid, char id, char speed);

// Returns immediatly, overwrites previous command if sent too quickly
int Reverse(int tid, char id);

// Blocks until sensor dump is requested and received
int GetSensors(int tid, char* buffer);

void tc_server(void);

#endif
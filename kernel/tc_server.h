#ifndef TC_SERVER_H
#define TC_SERVER_H

#define STRAIGHT 33
#define CURVED 34

#define INVALID_TC_SERVER -1
#define INVALID_ARGUMENT -2

#define TRAIN_COUNT 6
#define SWITCH_COUNT 22
#define MAX_TRAIN_NUMBER 79

// NOTE: This is an initial interface meant to satisfy the requirements 
//       for a0 and will expand/change during TC1 and TC2.

// Blocks until command is sent to the uart server
int SwitchTrack(int tid, char id, char dir);

// Return immediatly
int SwitchTrackAsync(int tid, char id, char dir);

// Returns immediatly
int SetSpeed(int tid, char id, char speed);

// Returns immediatly
int Reverse(int tid, char id);

// Blocks until sensor dump is requested and fully received
int GetSensors(int tid, char* buffer);

// Blocks until the tc server is finished initializing the track
int InitComplete(int tid);

void tc_server(void);

#endif
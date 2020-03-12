#ifndef TRAIN_CONTROL_SERVER_H
#define TRAIN_CONTROL_SERVER_H

#define INVALID_TRAIN_CONTROL_SERVER -1
#define INVALID_ARGUMENT -2
#define NO_PATH_EXISTS -3

#include "tc_server.h"
#include "track.h"
#include "train_state.h"

// NOTE: This is an initial interface meant to satisfy the requirements 
//       for TC1 and may expand/change for TC2.

typedef struct event_command EventCommand;

#define EVENT_SWITCH 1
#define EVENT_SPEED 2

struct event_command
{
    char valid;
    char type;
    char id;
    char arg;
};

// Offset in mm
int TargetPosition(int tid, char train_id, char track_node_number, int offset);

// Locate a train, and route it to reach a speed at a specific node
int InitTrain(int tid, char train_id, char original_track_node, char track_node_number, char speed_id);
// B9 -> E14 is 
// it 1 24 77 13 
// C13 -> A12 is 
// it 1 44 11 13
//

int SetPosition(int tid, char train_id, char sensor_id1, char sensor_id2);
int SetTrack(int tid, char track_id);
int GetPosition(int tid, char train_id, TrainState* tp);

void train_control_server(void);

int is_valid_speed(int speed_id);

#endif

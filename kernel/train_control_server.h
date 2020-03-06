#ifndef TRAIN_CONTROL_SERVER_H
#define TRAIN_CONTROL_SERVER_H

#define INVALID_TRAIN_CONTROL_SERVER -1
#define INVALID_ARGUMENT -2

#include "tc_server.h"
#include "track.h"

// NOTE: This is an initial interface meant to satisfy the requirements 
//       for TC1 and may expand/change for TC2.

typedef struct track_position TrackPosition;
typedef struct train_path_plan TrainPathPlan;

struct track_position
{
    int node;
    int offset;
};

struct train_path_plan
{
    TrackPosition pos;
    int path[TRACK_MAX];
    int path_distance[TRACK_MAX];
    int path_len;

    // Index in the path 
    int current_node;
};

// Offset in mm
int TargetPosition(int tid, char train_id, char track_node_number, int offset);
int SetPosition(int tid, char train_id, char sensor_id1, char sensor_id2);
int SetTrack(int tid, char track_id);
int GetPosition(int tid, char train_id, TrackPosition* tp);

void train_control_server(void);

#endif

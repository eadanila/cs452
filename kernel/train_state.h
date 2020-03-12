#ifndef PATH_PLAN_H
#define PATH_PLAN_H

#include "track.h"

#define TRAIN_SPEED_MAX 14
#define TRAIN_SPEED_MIN 8

typedef struct train_state TrainState;
typedef struct train_path_plan TrainPathPlan;

struct train_state
{
    int valid;

    int id;
    int node;
    int offset;
    int speed;
};

struct train_path_plan
{
    int valid;

    TrainState* state;
    int path[TRACK_MAX];
    int path_distance[TRACK_MAX];
    int path_len;

    // Index in the path 
    int current_node;
    int end_speed;
    int next_sensor_time;
};

void init_train_path_plan(TrainPathPlan* p, TrainState* train_state, 
                          int* previous, int* distances,
                          int dest, int dest_offset, char end_speed);
void init_train_state(TrainState* result, int id, int node, int offset, int speed);

// Returns next sensor in path, as index in the path plans path array.
int next_sensor(TrainPathPlan* plan, struct track_node* track);

int is_valid_speed(int speed_id);

#endif
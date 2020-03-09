#ifndef SWITCHES_H
#define SWITCHES_H

#define STRAIGHT 33
#define CURVED 34

#define TRAIN_COUNT 6
#define MAX_TRAIN_NUMBER 79
#define MAX_TRAIN_SPEED 14
#define MIN_TRAIN_SPEED 8
#define NO_SPEED -1

#define SWITCH_COUNT 22
#define MAX_SWITCH_NUMBER 0x9C

typedef struct track_constants TrackConstants;

struct track_constants
{
    int train_ids[TRAIN_COUNT];
    int train_id_to_index[MAX_TRAIN_NUMBER + 1]; // Array to convert a train_id to its position in "train_ids"
    
    int switch_ids[SWITCH_COUNT];
    int switch_id_to_index[MAX_SWITCH_NUMBER + 1];

    int stop_distance_a[MAX_TRAIN_NUMBER + 1][MAX_TRAIN_SPEED + 1];
    int real_speed_a[MAX_TRAIN_NUMBER + 1][MAX_TRAIN_SPEED + 1];

    int stop_distance_b[MAX_TRAIN_NUMBER + 1][MAX_TRAIN_SPEED + 1];
    int real_speed_b[MAX_TRAIN_NUMBER + 1][MAX_TRAIN_SPEED + 1];
};

TrackConstants create_track_constants();

#endif
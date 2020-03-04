// TODO Seperate labels into new layer

#ifndef TRACK_DISPLAY_H
#define TRACK_DISPLAY_H

#define TRACK_DISPLAY_HEIGHT 20
#define TRACK_DISPLAY_WIDTH 105

#define TURNOUT_UP 0
#define TURNOUT_RIGHT 1
#define TURNOUT_DOWN 2
#define TURNOUT_LEFT 3

#define TURNOUT_UP_RIGHT 4
#define TURNOUT_RIGHT_DOWN 5
#define TURNOUT_DOWN_LEFT 6
#define TURNOUT_LEFT_UP 7

#define TRACK_A 1
#define TRACK_B 2

typedef struct track_view TrackView;

struct track_view
{
    int height, width;
    int x, y;
    unsigned int** data; // Unicode track printout

    int* turnout_types;
    int* turnout_positions;
    int* turnout_to_index;
};

// Volatile is to avoid needing memcpy for defining track display arrays
void print_track(int tid, unsigned int** track, int x, int y);

// Move these attributes into a track object.
void toggle_switch(int tid, TrackView template_track, int turnout);

void init_track_view(TrackView* v, unsigned int** data, int height, int width, int x, int y, 
                     int* turnout_to_index,
                     int* turnout_types, 
                     int* turnout_positions);

#endif
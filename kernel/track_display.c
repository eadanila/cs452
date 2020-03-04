#include "track_display.h"
#include "terminal.h"
#include "print.h"
#include "uart.h"
#include "uart_server.h"
#include <bwio.h>
#include "logging.h"

void PutUni(int tid, unsigned int i)
{
    // Convert wide char to UTF-8
    
    if(i <= 0x7F) // 7 sig bits
    {
        Putc(tid, COM2, (char)(i));
    }
    else if(i <= 0x7FF) // 11 sig bits
    {
        Putc(tid, COM2, (char)(((i >> 6) & 0x1F) | 0xC0));
        Putc(tid, COM2, (char)((i & 0x3F) | 0x80));
    }
    else if(i <= 0xFFFF)
    {
        Putc(tid, COM2, (char)(((i >> 12) & 0xF) | 0xE0));
        Putc(tid, COM2, (char)(((i >> 6) & 0x3F) | 0x80));
        Putc(tid, COM2, (char)((i      & 0x3F  ) | 0x80));
    }
    else if(i <= 0x1FFFFF)
    {
        Putc(tid, COM2, (char)(((i >> 18) & 0x7 ) | 0xF0));
        Putc(tid, COM2, (char)(((i >> 12) & 0x3F) | 0x80));
        Putc(tid, COM2, (char)(((i >> 6)  & 0x3F) | 0x80));
        Putc(tid, COM2, (char)((i       & 0x3F  ) | 0x80));
    }
}

void PrintUni(int tid, unsigned int* s)
{
    while(*s)
    {
        PutUni(tid, *s);
        s++;
    }
}

void print_track(int tid, unsigned int** track, int x, int y)
{
    // MoveCursor(tid, x, y);
    for(int i = 0; i != TRACK_DISPLAY_HEIGHT; i++)
    {
        MoveCursor(tid, x, y + i);

        PrintUni(tid, track[i]);
        // Print(tid, "%x", track[i][0]);
        // Print( tid, track[i]);

    } 
}

void toggle_switch(int tid, TrackView template_track, int turnout)
{
    int t_index = template_track.turnout_to_index[turnout];
    int turnout_type = template_track.turnout_types[t_index];
    int turnout_x = template_track.turnout_positions[2*t_index + 1];
    int turnout_y = template_track.turnout_positions[2*t_index + 0];

    MoveCursor(tid, template_track.x + turnout_x, template_track.y + turnout_y);
    // print("%x ", template_track[turnout_y][ turnout_x]);
    PutUni(tid, template_track.data[turnout_y][ turnout_x]);
    switch(turnout_type)
    {
        case TURNOUT_UP:
            MoveCursor(tid, template_track.x + turnout_x, template_track.y + turnout_y - 1);
            PutUni(tid, template_track.data[turnout_y - 1][ turnout_x]);
            break;

        case TURNOUT_LEFT:
            MoveCursor(tid, template_track.x + turnout_x - 1, template_track.y + turnout_y);
            PutUni(tid, template_track.data[turnout_y][ turnout_x - 1]);
            break;

        case TURNOUT_DOWN:
            MoveCursor(tid, template_track.x + turnout_x, template_track.y + turnout_y + 1);
            PutUni(tid, template_track.data[turnout_y + 1][ turnout_x]);
            break;

        case TURNOUT_RIGHT:
            MoveCursor(tid, template_track.x + turnout_x + 1, template_track.y + turnout_y);
            PutUni(tid, template_track.data[turnout_y][ turnout_x + 1]);
            break;

        case TURNOUT_UP_RIGHT:
            MoveCursor(tid, template_track.x + turnout_x, template_track.y + turnout_y - 1);
            PutUni(tid, template_track.data[turnout_y - 1][ turnout_x]);
            MoveCursor(tid, template_track.x + turnout_x + 1, template_track.y + turnout_y);
            PutUni(tid, template_track.data[turnout_y][ turnout_x + 1]);
            break;

        case TURNOUT_RIGHT_DOWN:
            MoveCursor(tid, template_track.x + turnout_x + 1, template_track.y + turnout_y);
            PutUni(tid, template_track.data[turnout_y][ turnout_x + 1]);
            MoveCursor(tid, template_track.x + turnout_x, template_track.y + turnout_y + 1);
            PutUni(tid, template_track.data[turnout_y + 1][ turnout_x]);
            break;

        case TURNOUT_DOWN_LEFT:
            MoveCursor(tid, template_track.x + turnout_x, template_track.y + turnout_y + 1);
            PutUni(tid, template_track.data[turnout_y + 1][ turnout_x]);
            MoveCursor(tid, template_track.x + turnout_x - 1, template_track.y + turnout_y);
            PutUni(tid, template_track.data[turnout_y][ turnout_x - 1]);
            break;

        case TURNOUT_LEFT_UP:
            MoveCursor(tid, template_track.x + turnout_x - 1, template_track.y + turnout_y);
            PutUni(tid, template_track.data[turnout_y][ turnout_x - 1]);
            
            MoveCursor(tid, template_track.x + turnout_x, template_track.y + turnout_y - 1);
            PutUni(tid, template_track.data[turnout_y - 1][ turnout_x]);
            break;
        default:
            // For debug
            Print(tid, "%d ", turnout_type);
    }
    // Print(tid, "%d", turnout_type);
}

void init_track_view(TrackView* v, unsigned int** data, int height, int width, int x, int y, 
                     int* turnout_to_index,
                     int* turnout_types, 
                     int* turnout_positions)
{
    v->data = data;
    v->height = height;
    v->width = width;
    v->x = x;
    v->y = y;

    v->turnout_to_index = turnout_to_index;
    v->turnout_types = turnout_types;
    v->turnout_positions = turnout_positions;
}
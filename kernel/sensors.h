#ifndef SENSORS_H
#define SENSORS_H

#include "track_constants.h"

#define SENSOR_BANK_COUNT 5
#define SENSORS_PER_BANK 16
#define SENSOR_COUNT SENSOR_BANK_COUNT*SENSORS_PER_BANK
#define MAX_SENSOR_NUMBER 79

// Returns a unique index for a sensor to be used in an array
// that is SENSOR_COUNT long, based on the character representing it's
// sensor bank, and its index in that bank.
int sensor_index(char bank, int sensor_number);

// An inverse operation to sensor_index. Converts a unique sensor index
// into a sensor name of the form <CHAR><INT> such as B15
void sensor_name(int sensor_index, char* bank, int* sensor_number);

// Identical to sensor_index, but takes in a string representing the sensor
// bank and it's index in that bank.
int sensor_string_index(char* s);

// Identical to sensor_name, but returns the sensor name in a string instead.
// s must be of size 4
void sensor_name_string(int sensor_index, char* s);

// Parses a 10 byte sensor dump read from a Marklin box. sensor_states and 
// newly_triggered must be MAX_SENSOR_NUMBER long arrays. sensor_states should be an 
// array of booleans indiciating whether or not each sensor was previously
// active and will be updated based on the new sensor_bytes. newly_triggered
// will be populated with booleans indicating which sensor_states have gone from
// inactive to active.
// Require: sensor_bytes is a 10 byte long sensor dump
// Return: integer indicating whether or not any sensors have changed state
int parse_sensors(char* sensor_bytes, char* sensor_states, char* newly_triggered);

#endif
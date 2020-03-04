#include "sensors.h"
#include "logging.h"

int sensor_index(char bank, int sensor_number)
{
    assert( bank >= 'A' && bank <= 'E');
    assert( sensor_number >= 1 && sensor_number <= SENSORS_PER_BANK);

    return ((int)(bank - 'A'))*SENSORS_PER_BANK + (sensor_number - 1);
}

void sensor_name(int sensor_index, char* bank, int* sensor_number)
{
    assert(sensor_index >= 0 && sensor_index < SENSOR_COUNT);

    *bank = 'A' + sensor_index / SENSORS_PER_BANK;
    *sensor_number = sensor_index % SENSORS_PER_BANK + 1;
}

int parse_sensors(char* sensor_bytes, char* sensor_states, char* newly_triggered)
{
	int updated = 0;

    for(int i = 0; i != SENSOR_COUNT; i++) newly_triggered[i] = 0;

	for( int i = 0; i != 10; i+=2)
	{
		char byte1 = sensor_bytes[i];
		char byte2 = sensor_bytes[i+1];
		int data = byte1;
		data = data << 8;
		data += byte2;

		int sensor_bank = i/2;

		for(int i = SENSORS_PER_BANK; i != 0; i--)
		{
			int sensor = sensor_bank * SENSORS_PER_BANK + (i-1);
			if(data & 1) 
			{
				if(!sensor_states[sensor])
				{
					updated = 1;
					// add_sensor_to_queue(sensor_bank * 16 + (i-1)); 
                    newly_triggered[sensor] = 1;
					sensor_states[sensor] = 1;
				}
			}
			else
			{
				sensor_states[sensor] = 0;
			}
			data = data >> 1;
		}
	}

	return updated;
}
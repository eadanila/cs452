#include "track_constants.h"
#include "memcpy.h"

TrackConstants create_track_constants()
{
    TrackConstants result;

    // Initialize train constants
    int train_ids[TRAIN_COUNT] = {1, 24, 58, 74, 78, 79}; // All possible train id's
    memcpy(result.train_ids, train_ids, TRAIN_COUNT*sizeof(int));
    for(int i = 0; i != MAX_TRAIN_NUMBER + 1; i++) result.train_id_to_index[i] = -1; // Initialize train_to_enumeration
    for(int i = 0; i != TRAIN_COUNT; i++) result.train_id_to_index[train_ids[i]] = i; // Initialize train_to_enumeration

    // Initialize switch states
	for(int i = 0; i != 18; i++) result.switch_ids[i] = i + 1;
	result.switch_ids[18] = 0x99;
	result.switch_ids[19] = 0x9A;
	result.switch_ids[20] = 0x9B;
	result.switch_ids[21] = 0x9C;
	// for(int i = 0; i != 256; i++) switch_states[i] = 0;
	// for(int i = 0; i != SWITCH_COUNT; i++) switch_states[switches[i]] = 'S';
	// switch_updated = 1;

	for(int i = 1; i != 19; i++) result.switch_id_to_index[i] = i - 1;
	result.switch_id_to_index[0x99] = 18;
	result.switch_id_to_index[0x9A] = 19;
	result.switch_id_to_index[0x9B] = 20;
	result.switch_id_to_index[0x9C] = 21;

	// Real speeds and stopping distances

	for(int train = 0; train != MAX_TRAIN_NUMBER + 1; train++)
	{
		for(int speed = 0; speed != MAX_TRAIN_SPEED + 1; speed++)
		{
			result.stop_distance_a[train][speed] = NO_SPEED;
			result.real_speed_a[train][speed] = NO_SPEED;
			
			result.stop_distance_b[train][speed] = NO_SPEED;
			result.real_speed_b[train][speed] = NO_SPEED;
		}
	}

	for(int train = 0; train != MAX_TRAIN_NUMBER + 1; train++)
	{
		// For now, set all trains to the same for both tracks

		result.stop_distance_a[train][MIN_TRAIN_SPEED] = 400;
		result.stop_distance_a[train][MAX_TRAIN_SPEED] = 1000;
		result.real_speed_a[train][MIN_TRAIN_SPEED] = 333; // mm/s
		result.real_speed_a[train][MAX_TRAIN_SPEED] = 641; // mm/s

		result.stop_distance_b[train][MIN_TRAIN_SPEED] = 400;
		result.stop_distance_b[train][MAX_TRAIN_SPEED] = 1000;
		result.real_speed_b[train][MIN_TRAIN_SPEED] = 333; // mm/s
		result.real_speed_b[train][MAX_TRAIN_SPEED] = 641; // mm/s
	}

    return result;
}
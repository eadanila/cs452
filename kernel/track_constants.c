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

	// for(int train = 0; train != MAX_TRAIN_NUMBER + 1; train++)
	// {
	// 	// For now, set all trains to the same for both tracks

	// 	result.stop_distance_a[train][MIN_TRAIN_SPEED] = 400;
	// 	result.stop_distance_a[train][MAX_TRAIN_SPEED] = 1000;
	// 	result.real_speed_a[train][MIN_TRAIN_SPEED] = 333; // mm/s
	// 	result.real_speed_a[train][MAX_TRAIN_SPEED] = 641; // mm/s

	// 	result.stop_distance_b[train][MIN_TRAIN_SPEED] = 400;
	// 	result.stop_distance_b[train][MAX_TRAIN_SPEED] = 1000;
	// 	result.real_speed_b[train][MIN_TRAIN_SPEED] = 333; // mm/s
	// 	result.real_speed_b[train][MAX_TRAIN_SPEED] = 641; // mm/s
	// }

	int track_a_length = 4867; // mm
    int track_b_length = 4698; // mm

    // track a stop distances
    result.stop_distance_a[74][8]  = 460;
    result.stop_distance_a[74][11] = 690;
    result.stop_distance_a[74][14] = 840;

    result.stop_distance_a[78][8]  = 145;
    result.stop_distance_a[78][11] = 420;
    result.stop_distance_a[78][14] = 920;

    result.stop_distance_a[24][8]  = 210;
    result.stop_distance_a[24][11] = 590;
    result.stop_distance_a[24][14] = 1375;

    result.stop_distance_a[58][8]  = 190;
    result.stop_distance_a[58][11] = 550;
    result.stop_distance_a[58][14] = 1250;

    result.stop_distance_a[1][8]  = 200;
    result.stop_distance_a[1][11] = 575;
    result.stop_distance_a[1][14] = 1260;

    result.stop_distance_a[79][8]  = 220;
    result.stop_distance_a[79][11] = 620;
    result.stop_distance_a[79][14] = 1290;


    // track b stop distances
    result.stop_distance_b[24][8]  = 205;
    result.stop_distance_b[24][11] = 598;
    result.stop_distance_b[24][14] = 1268;

    result.stop_distance_b[79][8]  = 225;
    result.stop_distance_b[79][11] = 625;
    result.stop_distance_b[79][14] = 1275;

    result.stop_distance_b[58][8]  = 178;
    result.stop_distance_b[58][11] = 568;
    result.stop_distance_b[58][14] = 1258;

    result.stop_distance_b[74][8]  = 470;
    result.stop_distance_b[74][11] = 570;
    result.stop_distance_b[74][14] = 1260;

    result.stop_distance_b[78][8]  = 140;
    result.stop_distance_b[78][11] = 420;
    result.stop_distance_b[78][14] = 920;

    result.stop_distance_b[1][8]  = 200;
    result.stop_distance_b[1][11] = 570;
    result.stop_distance_b[1][14] = 1255;


    // track a global average speeds
    result.real_speed_a[74][8]  = track_a_length*1000/(12948);
    result.real_speed_a[74][11] = track_a_length*1000/(9075);
    result.real_speed_a[74][14] = track_a_length*1000/(7780);

    result.real_speed_a[78][8]  = track_a_length*1000/(28639);
    result.real_speed_a[78][11] = track_a_length*1000/(15016);
    result.real_speed_a[78][14] = track_a_length*1000/(9800);

    result.real_speed_a[24][8]  = track_a_length*1000/(22500);
    result.real_speed_a[24][11] = track_a_length*1000/(12040);
    result.real_speed_a[24][14] = track_a_length*1000/(8570);

    result.real_speed_a[58][8]  = track_a_length*1000/(25800);
    result.real_speed_a[58][11] = track_a_length*1000/(13220);
    result.real_speed_a[58][14] = track_a_length*1000/(8220);

    result.real_speed_a[1][8]  = track_a_length*1000/(23200);
    result.real_speed_a[1][11] = track_a_length*1000/(12300);
    result.real_speed_a[1][14] = track_a_length*1000/(8300);

    result.real_speed_a[79][8]  = track_a_length*1000/(19600);
    result.real_speed_a[79][11] = track_a_length*1000/(11100);
    result.real_speed_a[79][14] = track_a_length*1000/(7300);


    // track b global average sppeds
    result.real_speed_b[24][8]  = track_b_length*1000/(21600);
    result.real_speed_b[24][11] = track_b_length*1000/(11500);
    result.real_speed_b[24][14] = track_b_length*1000/(8200);

    result.real_speed_b[79][8]  = track_b_length*1000/(18900);
    result.real_speed_b[79][11] = track_b_length*1000/(10700);
    result.real_speed_b[79][14] = track_b_length*1000/(7000);

    result.real_speed_b[58][8]  = track_b_length*1000/(24800);
    result.real_speed_b[58][11] = track_b_length*1000/(12700);
    result.real_speed_b[58][14] = track_b_length*1000/(7900);

    result.real_speed_b[74][8]  = track_b_length*1000/(12400);
    result.real_speed_b[74][11] = track_b_length*1000/(8720);
    result.real_speed_b[74][14] = track_b_length*1000/(7400);

    result.real_speed_b[78][8]  = track_b_length*1000/(27550);
    result.real_speed_b[78][11] = track_b_length*1000/(14540);
    result.real_speed_b[78][14] = track_b_length*1000/(9545);

    result.real_speed_b[1][8]  = track_b_length*1000/(22420);
    result.real_speed_b[1][11] = track_b_length*1000/(11940);
    result.real_speed_b[1][14] = track_b_length*1000/(7990);

    return result;
}
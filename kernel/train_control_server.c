#include "train_control_server.h"
#include "tc_server.h"
#include "string_utility.h"
#include "syscall.h"
#include "name_server.h"
#include "track.h"
#include "logging.h"
#include "sensors.h"
#include "timer.h"
#include "track_constants.h"
#include "sensors.h"
#include "terminal.h"
#include "clock_server.h"
#include "shortest_path.h"

#define TARGET_POSITION 1
#define SET_POSITION 2
#define GET_POSITION 3
#define SET_TRACK 4
#define INIT_TRAIN 8

#define SENSOR_DUMP 5
#define NEW_TIME 6
#define EVENT_OCCURED 7

#define TRACK_A_SIZE 144
#define TRACK_B_SIZE 140

#define STATUS_BAR_HEIGHT 43

#define MAX_COMMAND_NUMBER MAX_TASKS_ALLOWED - 1

void create_event_command(int ticks, int type, int id, int arg);

int TargetPosition(int tid, char train_id, char track_node_number, int offset)
{
    char message[7];
    char reply[1];

    message[0] = TARGET_POSITION;
    message[1] = train_id;
    message[2] = track_node_number;
    pack_int(offset, message + 3);

    // If tid is not the task id of an existing task.
    if(Send(tid, message, 7, reply, 0) == -1) 
        return INVALID_TC_SERVER;

    return 0;
}

int InitTrain(int tid, char train_id, char original_track_node, char track_node_number, char speed_id)
{
    char message[5];
    char reply[1];

    message[0] = INIT_TRAIN;
    message[1] = train_id;
    message[2] = original_track_node;
    message[3] = track_node_number;
    message[4] = speed_id;

    // If tid is not the task id of an existing task.
    if(Send(tid, message, 5, reply, 1) == -1) 
        return INVALID_TRAIN_CONTROL_SERVER;

    return (signed char)reply[0];
}

int SetPosition(int tid, char train_id, char sensor_id1, char sensor_id2)
{
    return 0;
}

int GetPosition(int tid, char train_id, TrainState* tp)
{
    return 0;
}

int SetTrack(int tid, char track_id)
{
    char message[2];
    char reply[1];

    message[0] = SET_TRACK;
    message[1] = track_id;

    // If tid is not the task id of an existing task.
    if(Send(tid, message, 2, reply, 0) == -1) 
        return INVALID_TC_SERVER;

    return 0;
}

// State variables, not accessed by any tasks other than train_control_server process and functions
// which only train_control_server calls.
static char track_id;
static track_node* current_track;
static int (*shortest_paths)[TRACK_MAX];
static int (*shortest_distances)[TRACK_MAX];
static TrackConstants track_constants;

static int tcid;
static int pid;
static track_node tracka[TRACK_MAX];
static track_node trackb[TRACK_MAX];

// For each node, a complete path 
int shortest_paths_a[TRACK_MAX][TRACK_MAX];
int shortest_paths_b[TRACK_MAX][TRACK_MAX];

// Distances for above paths
int shortest_distances_a[TRACK_MAX][TRACK_MAX];
int shortest_distances_b[TRACK_MAX][TRACK_MAX];

EventCommand event_commands[MAX_COMMAND_NUMBER + 1];

// TODO In the future these will be arrays.
static TrainState train_state;
static TrainPathPlan plan;

static int event_cnt;
static int sensor_time;

void _set_position(char train_id, char sensor_id1, char sensor_id2)
{

}

void _target_position(char train_id, char track_node_number, int offset)
{
    // Train id currently ignored since only one train
    assert(train_id == train_state.id);

    // Overwrite the current track plan
    init_train_path_plan(&plan, &train_state, 
                         shortest_paths[train_state.node], shortest_distances[train_state.node],
                         track_node_number, offset, 0);
}

void _get_position(char train_id)
{

}

int real_speed(int train, int speed)
{
    if(track_id == 'A') return track_constants.real_speed_a[train][speed];
    if(track_id == 'B') return track_constants.real_speed_b[train][speed];
    assert(0); //Shouldn't get here!
    return 0;
}

int stopping_distance(int train, int speed)
{
    if(track_id == 'A') return track_constants.stop_distance_a[train][speed];
    if(track_id == 'B') return track_constants.stop_distance_b[train][speed];
    assert(0); //Shouldn't get here!
    return 0;
}

// Compute all shortest paths between all nodes and their distances
void compute_shortest_paths()
{
    // Takes roughly 750 ms
    // int start = read_debug_timer();
    for(int i = 0; i != TRACK_A_SIZE; i++) 
        shortest_path(tracka, TRACK_A_SIZE, i, shortest_distances_a[i], shortest_paths_a[i]);

    for(int i = 0; i != TRACK_B_SIZE; i++) 
        shortest_path(trackb, TRACK_B_SIZE, i, shortest_distances_b[i], shortest_paths_b[i]);
    // print(" time %d ", read_debug_timer() - start);
}

void initialize()
{
    for(int i = 0; i != TRACK_MAX; i++)
    {
        for(int j = 0; j != TRACK_MAX; j++)
        {
            shortest_paths_a[i][j] = UNREACHABLE;
            shortest_paths_b[i][j] = UNREACHABLE;
            shortest_distances_a[i][j] = INFINITY;
            shortest_distances_b[i][j] = INFINITY;
        }
    } 

    init_tracka(tracka);
    init_trackb(trackb);

    compute_shortest_paths();
}

// Continously spins for sensor data from the tc server and sends it 
// to the train control server. 
void train_control_sensor_state_notifier()
{
    // SHARED MEMORY FIX
	int tcid = WhoIs("tc_server");
	int tid = WhoIs("train_control");

	char msg[11];
	char reply[1];

    msg[0] = SENSOR_DUMP;

    InitComplete(tcid);

	for(;;)
	{
		GetSensors(tcid, msg + 1);
        // panic();
		// Notify server that a sensor dump was completed
        Send(tid, msg, 11, reply, 0);
	}
}

void event()
{
    int cid = WhoIs("clock_server");
    int sender;
    char msg[4];

    // Receive integer delay and respond instantly
    Receive(&sender, msg, 4);
    Reply(sender, msg, 0);

    // Delay for time received
    int time = Delay(cid, unpack_int(msg));

    // After the delay, send back the current time!
    char response[5];
    response[0] = EVENT_OCCURED;
    pack_int(time, response + 1);
    char reply;
    Send(sender, response, 5, &reply, 0);
}

// // This is used for interpolation, and executing commands at any tick.
// // Sends the current time to the tc_server ever tick.
// void time_notifier()
// {
//     int tcid = WhoIs("train_control");
//     int csid = WhoIs("clock_server");

//     char message[5];
//     char reply[1];

//     message[0] = NEW_TIME;

//     for(;;)
//     {
//         int time = Delay(csid, 5);
//         pack_int(time, message + 1);

//         // Notify server that time has changed
//         Send(tcid, message, 5, reply, 0);
//     }
// }

// TODO Remove these pointers, the function now switches tracks
int switch_needed(TrainPathPlan* p)
{
    char switch_id;
    char direction;
    // Switch all switches between the next two sensors!
    // Look if there is a branch between the next two sensors, switch if there is.
    // If branch is last node in the path, we say no switch is needed.

    int result = 0;

    int sensors_past = 0;

    // On the last node!
    if(p->current_node == p->path_len - 1) return 0;

    for(int i = p->current_node + 1; i != p->path_len; i++)
    {
        if(current_track[p->path[i]].type == NODE_SENSOR)
        {
            sensors_past++;
            if(sensors_past == 2) return 0;
        }

        track_node* node = &current_track[p->path[i]];
        if(sensors_past == 1 && node->type == NODE_BRANCH && i != p->path_len-1)
        {
            track_node* after_branch = &current_track[p->path[i+1]];

            switch_id = node->num;

            if(node->edge[DIR_STRAIGHT].dest == after_branch) direction = STRAIGHT;
            else if(node->edge[DIR_CURVED].dest == after_branch) direction = CURVED; 

            SwitchTrackAsync(tcid, switch_id, direction);
            TPrintAt(pid, 0, 40, "switched %d in dir %d ", switch_id, direction);

            result = 1;
        }
    }

    return result;
}

// TODO Move somewhere?
int is_sensor(int node_id)
{
    return (node_id >= 0 && node_id < MAX_SENSOR_NUMBER);
}

void clear_status_print()
{
    TPrintAt(pid, 30, STATUS_BAR_HEIGHT, "TIME DEVIATION: -    ");
    TPrintAt(pid, 60, STATUS_BAR_HEIGHT, "DISTANCE DEVIATION: -    ");
    TPrintAt(pid, 1, STATUS_BAR_HEIGHT, "NEXT SENSOR ON PATH: -    ");
}

void update_path_plan(TrainState* train_state, char* newly_triggered)
{
    // train_state has been updated, but path has not.
    
    assert(plan.state == train_state);

    // next sensor in the outdated path
    int next = next_sensor(&plan, current_track);
    if(next != -1) TPrintAt(pid, 0, 45, "next sensor %d ", plan.path[next]);

    // We should not be running if the last sensor in the path has already been hit
    // This means we were trying to reloop while the track was not in loop. Keep going and 
    // assert(next != -1); // CHANGED THIS

    int train = train_state->id;
    int speed = train_state->speed;
    
    // If train has deviated, replan
    // This can be if next does not match
    if(next == -1 || train_state->node != plan.path[next]) 
    {
        // If the train has deviated from the path, replan. 
        int dest = plan.path[plan.path_len-1];
        int offset = plan.path_distance[plan.path_len-1];
        char end_speed = plan.end_speed;
        
        // For TC1, assume that a path still exists to the planned destination 
        // (as it always does when in the centermost loop)
        assert(shortest_paths[train_state->node][dest] != UNREACHABLE);

        init_train_path_plan(&plan, train_state, 
                             shortest_paths[train_state->node], shortest_distances[train_state->node], 
                             dest, offset, end_speed);
        
        TPrintAt(pid, 0, 51, "Missed exit, new route created from %d to %d ", train_state->node, dest);
        return;
    }

    // If we are on track, but dont have enough distance to stop, continue looping.
    // Allow it to deviate so it may replan later.
    if(plan.end_speed == 0 && plan.path_distance[next] < stopping_distance(train, speed))
    {
        return;
    }

    // Update current node in path plan if this sensor hit was on the path.
    plan.current_node = next;
    assert(plan.path[plan.current_node] == train_state->node)  // Should now match train state.

    // Make next the future sensor to be hit now.
    next = next_sensor(&plan, current_track);

    // update next sensor time if there is another sensor!
    if(next != -1)
    {
        if(plan.next_sensor_time != -1)
        {
            TPrintAt(pid, 30, STATUS_BAR_HEIGHT, "TIME DEVIATION: %d    ",  sensor_time - plan.next_sensor_time );
            TPrintAt(pid, 60, STATUS_BAR_HEIGHT, "DISTANCE DEVIATION: %d    ",  
                    ((sensor_time - plan.next_sensor_time) * real_speed(train, speed))/100 );
        }

        char sensor_string[5];
        sensor_name_string(plan.path[next], sensor_string);
        TPrintAt(pid, 1, STATUS_BAR_HEIGHT, "NEXT SENSOR ON PATH: %s    ",  sensor_string);
    }

    int next_sensor_distance = (plan.path_distance[plan.current_node] - plan.path_distance[next]);
    int next_sensor_ticks = (next_sensor_distance*100)/real_speed(train, speed);
    plan.next_sensor_time = next_sensor_ticks + sensor_time;

    // Switch a turnout if needed
    // TODO Keep track of global switch state and transition to that 
    switch_needed(&plan);

    // No new sensors -> invalidate the track plan, calculate everything else needed for the path
    if(next == -1)
    {
        plan.valid = 0;

        clear_status_print();
    }

    // Check if its time to stop
    // Stop if were on the last sensor or the next sensors distance to the goal is less than our stopping distance
    // (Our must be greater since we always check the next like this)
    if(plan.end_speed == 0 && (next == -1 || plan.path_distance[next] < stopping_distance(train, speed)))  // TODO <= or < ?
    {
        plan.valid = 0; // invalidate path plan, we have reached the destination
        // Create an event to stop when its time.
        assert(plan.path_distance[plan.current_node] >= stopping_distance(train, speed));

        int stop_command_distance = (plan.path_distance[plan.current_node] - stopping_distance(train, speed));
        int stop_command_ticks = (stop_command_distance*100)/real_speed(train, speed);
                                 // *100 since 100 ticks in a second, to get number of ticks instead of seconds 
                                 // mm / ( (mm/s)*100 )

        create_event_command(stop_command_ticks, EVENT_SPEED, train, 0);
    }
}

// In the future this will be an array of train_states
// Does not interpolate, simple snaps train into position based on sensor hits.
// Return if any sensor was NEWLY TRIGGERED
void update_train_states(TrainState* train_state, char* newly_triggered)
{
    int sensor_triggered = -1;
    // For now assume only one train can trigger.  In the future will use train states distances
    // from each sensor to figure out which train state to update.

    // Find newly triggered sensor
    for(int i = 0; i != MAX_SENSOR_NUMBER + 1; i++)
    {
        if(newly_triggered[i]) 
        {
            sensor_triggered = i;
            break;
        }
    }

    // No new sensors triggered, dont update states based on sensor
    if(sensor_triggered == -1) return;

    // Update train position to this sensor
    train_state->node = sensor_triggered;
}

int create_event(int delay)
{
    assert(delay > 0);
    // Create event task
    int event_id = Create(3, event);
    char msg[4];
    char reply[1];

    // Send the delay duration
    pack_int(delay, msg);
    Send(event_id, msg, 5, reply, 0); // Will get replied to instantly 

    // return the tid of the task as the event id
    return event_id;
}

void create_event_command(int ticks, int type, int id, int arg)
{
    int event_id = create_event(ticks);

    event_commands[event_id].type = type;
    event_commands[event_id].id = id;
    event_commands[event_id].arg = arg;
    event_commands[event_id].valid = 1;
}

void execute_event(int event_id)
{
    // Currently ignore event id, assume all events are to stop

    assert(event_commands[event_id].valid);

    TPrintAt(pid, 0, 42, "event %d occured of type %d ", event_id, event_commands[event_id].type);

    switch(event_commands[event_id].type)
    {
        case EVENT_SWITCH:
            // TODO Make blocking switch?
            SwitchTrackAsync(tcid, event_commands[event_id].id, event_commands[event_id].arg);
            break;
        case EVENT_SPEED:
            SetSpeed(tcid, event_commands[event_id].id, event_commands[event_id].arg);
            break;
    }

    event_commands[event_id].valid = 0;
}

// Initialize the trains state, 
void _init_train(char train_id, char original_track_node, char track_node_number, char speed_id)
{
    // In TC2, use train id to select the train state and path plan from a list for each train.
    init_train_state(&train_state, train_id, original_track_node, 0, speed_id);
    init_train_path_plan(&plan, &train_state, 
                         shortest_paths[train_state.node], shortest_distances[train_state.node], 
                         track_node_number, 0, speed_id);

    SetSpeed(tcid, train_id, speed_id);

    TPrintAt(pid, 0, 44, "INIT_TRAIN");
}

void train_control_server(void)
{
    event_cnt = 0; event_cnt += 0;
    RegisterAs("train_control");

    tcid = WhoIs("tc_server");
    int cid = WhoIs("clock_server");
    pid = WhoIsWait(cid, "terminal_output");

    Create(3, train_control_sensor_state_notifier);
    // Create(3, time_notifier);

    int sender;
    char msg[11];
    char reply_msg[1];
    reply_msg[0] = 0;

    track_constants = create_track_constants();

    // State variables

    initialize();
    current_track = tracka;
    track_id = 'A';
    shortest_paths = shortest_paths_a;
    shortest_distances = shortest_distances_a;

    // Initialize single train state 
    train_state.valid = 0;
    plan.valid = 0;

    // Parsed sensor states 
    char sensor_states[MAX_SENSOR_NUMBER + 1];
    char newly_triggered_sensors[MAX_SENSOR_NUMBER + 1];

    for(int i = 0; i != MAX_COMMAND_NUMBER + 1; i++) event_commands[i].valid = 0;

    int event_time = Time(cid); event_time += 0; //Suppress pedantic
    sensor_time = event_time;
    int new_event_time;
    int sensor_updated = 0;

    // // Verify path is correct.
    // print("path length %d", plan.path_len);
    // for(int i = 0; i != plan.path_len; i++) 
    // {
    //     // print("type: %d\n\r", (int)track[plan.path[i]].type);
    //     if(track[plan.path[i]].type == NODE_SENSOR) print("%s ", track[plan.path[i]].name);
    // }
    int cnt = 0; cnt += 0;
    // create_event_command(800, EVENT_SWITCH, 1, CURVED); //test event

    clear_status_print();

    for(;;)
    {
        // Receive clock server request
        Receive(&sender, msg, 11);

        // Execute the command encoded in the request
        switch(msg[0])
        {
            // case NEW_TIME:
            //     // Interpolate and possible produce actions

            //     new_time = unpack_int(msg + 1);
            //     // Delta time should always be 1, but is calculated for sanity.
            //     interpolate_train_states(&train_state, new_time - time);

            //     time = new_time;

            //     Reply(sender, reply_msg, 0);
            //     break;

            case EVENT_OCCURED:
                new_event_time = unpack_int(msg + 1);
                
                // The event id is the tid of the event sender
                execute_event(sender);
                event_time = new_event_time;

                Reply(sender, reply_msg, 0);
                break;
                
            case SET_TRACK:
                track_id = msg[1];
                if(track_id == 'A')
                {
                    current_track = tracka;
                    shortest_paths = shortest_paths_a;
                    shortest_distances = shortest_distances_a;
                } 
                else if(track_id == 'B')
                {
                    current_track = trackb;
                    shortest_paths = shortest_paths_b;
                    shortest_distances = shortest_distances_b;
                } 
                Reply(sender, reply_msg, 0);
                break;

            case TARGET_POSITION:
                assert(shortest_paths[train_state.node][(unsigned int)msg[2]] != UNREACHABLE)
                Reply(sender, reply_msg, 0);
                _target_position(msg[1], msg[2], unpack_int(msg + 3));
                break;
            case SET_POSITION:

                Reply(sender, reply_msg, 0);
                break;

            case SENSOR_DUMP:
                newly_triggered_sensors[0] += 0;
                sensor_states[0] += 0;
                sensor_updated = parse_sensors(msg + 1, sensor_states, newly_triggered_sensors);
                
                if(train_state.valid) update_train_states(&train_state, newly_triggered_sensors);
                // Only update path plan if sensor was newly triggered!
                sensor_updated += 0;
                if(plan.valid && train_state.valid && sensor_updated)
                {
                    sensor_time = Time(cid);
                    update_path_plan(&train_state, newly_triggered_sensors);
                } 

                Reply(sender, reply_msg, 0);
                break;

            case INIT_TRAIN:
                // Check if path exists
                assert(shortest_paths == shortest_paths_a || shortest_paths == shortest_paths_b);
                if(shortest_paths[(unsigned int)msg[2]][(unsigned int)msg[3]] == UNREACHABLE)
                {
                    reply_msg[0] = NO_PATH_EXISTS;
                    Reply(sender, reply_msg, 1);
                } 
                else
                { 
                    reply_msg[0] = 0;
                    Reply(sender, reply_msg, 1); // Temp reply first to allow prints to terminal
                    _init_train(msg[1], msg[2], msg[3], msg[4]);
                }

                break;
        }

        // TPrintAt(pid, 0, 40, "train_location: %s   ", track[train_state.node].name);
        // TPrintAt(pid, 0, 41, "cnt: %d", cnt++);
    }
}
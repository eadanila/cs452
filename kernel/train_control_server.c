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

#define TARGET_POSITION 1
#define SET_POSITION 2
#define GET_POSITION 3
#define SET_TRACK 4

#define SENSOR_DUMP 5

#define INFINITY __INT_MAX__
#define UNREACHABLE -1
#define NEIGHBOR_MAX 3

#define TRACK_A_SIZE 144
#define TRACK_B_SIZE 140

#define TRAIN_SPEED_MAX 13
#define TRAIN_SPEED_MIN 8

int TargetPosition(int tid, char train_id, char track_node_number, int offset)
{
    char message[9];
    char reply[1];

    message[0] = SET_TRACK;
    message[1] = train_id;
    message[2] = train_id;
    pack_int(offset, message + 3);;

    // If tid is not the task id of an existing task.
    if(Send(tid, message, 9, reply, 0) == -1) 
        return INVALID_TC_SERVER;

    return 0;
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
char track_id;
track_node* track;
TrackConstants track_constants;

int tcid;
track_node tracka[TRACK_MAX];
track_node trackb[TRACK_MAX];

// For each node, a complete path 
int shortest_paths_a[TRACK_MAX][TRACK_MAX];
int shortest_paths_b[TRACK_MAX][TRACK_MAX];

// Distances for above paths
int shortest_distances_a[TRACK_MAX][TRACK_MAX];
int shortest_distances_b[TRACK_MAX][TRACK_MAX];

// UNCOMMENT FOR STACK ALLOCATED SHORTEST PATH DATA
    // track_node* tracka;
    // track_node* trackb;

    // // For each node, a complete path 
    // int (*shortest_paths_a)[TRACK_MAX];
    // int (*shortest_paths_b)[TRACK_MAX];

    // // Distances for above paths
    // int (*shortest_distances_a)[TRACK_MAX];
    // int (*shortest_distances_b)[TRACK_MAX];

void _set_position(char train_id, char sensor_id1, char sensor_id2)
{

}

void _target_position(char train_id, char track_node_number, int offset)
{

}

void _get_position(char train_id)
{

}

// helpers for dijkstras

int link_cost(track_node* track, int u, int v) // u -> v
{
    // BR and MR are reverse to eachother
    // SENSORs are reverse of their opposite direction SENSOR
    // ENTER AND EXIT are reverse of eachother

    int cost = INFINITY;

    switch(track[u].type)
    {
        case NODE_EXIT: // No edges
            // if(track + v == track[u].reverse) 
            //     // Reverse will be an ENTER
            //     return track[u].reverse->edge[DIR_AHEAD].dist;
            break;

        case NODE_SENSOR:
        case NODE_MERGE:
        case NODE_ENTER: // 1 edge
            if(track + v == track[u].edge[DIR_AHEAD].dest) 
                cost = track[u].edge[DIR_AHEAD].dist;

            // if(track + v == track[u].reverse) 
            //     // The reverse track may be any type of track
            //     // reverse order of u, v and return that dist instead.
            //     // If two nodes are reverse of eachother, we have two directions 
            //     //   of the same sensor, in which case we make the dist 0

            //     if(track + v == track[u].reverse) return 0;

            //     link_cost(track, v, u);
            break;

        case NODE_BRANCH: // 2 edges
            if(track + v == track[u].edge[DIR_STRAIGHT].dest) 
                cost = track[u].edge[DIR_STRAIGHT].dist;

            if(track + v == track[u].edge[DIR_CURVED].dest) 
                cost = track[u].edge[DIR_CURVED].dist;

            // if(track + v == track[u].reverse) 
            //     // The reverse track may be any type of track
            //     // reverse order of u, v and return that dist instead.
            //     link_cost(track, v, u);
            break;

        case NODE_NONE:
            assert(0);
            break;
    }

    // print("link end");
    return cost;
}

void get_neighbors(track_node* track, int u, int* neighbors, int* size)
{
    *size = 0;
    switch(track[u].type)
    {
        case NODE_EXIT: // No edges
            // neighbors[0] = (int)(track[u].reverse - track);
            // *size += 1;
            return;
            break;

        case NODE_SENSOR:
        case NODE_MERGE:
        case NODE_ENTER: // 1 edge
            neighbors[0] = (int)(track[u].edge[DIR_AHEAD].dest - track);
            // neighbors[1] = (int)(track[u].reverse - track);
            *size += 1;
            return;
            break;

        case NODE_BRANCH: // 2 edges
            neighbors[0] = (int)(track[u].edge[DIR_STRAIGHT].dest - track); 
            neighbors[1] = (int)(track[u].edge[DIR_CURVED].dest - track);
            // neighbors[2] = (int)(track[u].reverse - track);
            *size += 2;
            return;
            break;

        case NODE_NONE:
            assert(0);
            break;
    }
}

int is_contained(int element, int* list, int size)
{
    for(int i = 0; i != size; i++) if(list[i] == element) return 1;
    return 0;
}

// distance and previous must both be TRACK_MAX size arrays
// NOTE: Assumes switches may be switched to obtain shortest path.
void shortest_path(track_node* track, const int track_node_count, int source, int* distance, int* previous)
{
    // Initialize distance and previous arrays
    for(int i = 0; i != TRACK_MAX; i++)
    {
        distance[i] = INFINITY;
        previous[i] = UNREACHABLE;
    } 

    int N_visited[TRACK_MAX];

    for(int i = 0; i != TRACK_MAX; i++) N_visited[i] = 0;
    N_visited[source] = 1;
    int N_size = 1;

    int neighbors[NEIGHBOR_MAX];
    int neighbors_size = 0;

    get_neighbors(track, source, neighbors, &neighbors_size);

    for(int i = 0; i != neighbors_size; i++) distance[neighbors[i]] = link_cost(track, source, neighbors[i]);

    // Exit piece chosen as source.
    if(neighbors_size == 0) return;
    
    for(;;)
    {
        int w = UNREACHABLE;
        int w_dist = INFINITY;

        // Find w not in N' with minimum D(u,w)
        for(int i = 0; i != track_node_count; i++)
        {
            // N[i] == 0 means i is not contained in N
            if(N_visited[i] == 0 && distance[i] <= w_dist )
            {
                w = i;
                w_dist = distance[i];
            }
        }
        
        assert(w != UNREACHABLE);

        // Add w to N'
        N_visited[w] = 1;
        N_size++;

        int w_neighbors[NEIGHBOR_MAX];
        int w_neighbors_size;
        get_neighbors(track, w, w_neighbors, &w_neighbors_size);

        // For all v not in N' and adjacent to w
        for(int i = 0; i != w_neighbors_size; i++)
        {
            int v = w_neighbors[i];
            
            // If v is contained in N
            if(N_visited[v] == 1) continue;

            int w_to_v_cost = link_cost(track, w, v);
            if(distance[w] + w_to_v_cost < distance[v])
            {
                distance[v] = distance[w] + w_to_v_cost;
                previous[v] = w;
            }
        }

        // If all nodes visited, we're done.
        if(N_size == track_node_count) break;
    } 
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
    init_tracka(tracka);
    init_trackb(trackb);

    compute_shortest_paths();
}

// Train path functions

// NOTE: Assuming dest_offset does not reach the node before or after it.
// TODO Add an error return for this ^
// A shortest path from node to dest must exist.
void init_train_path_plan(TrainPathPlan* p, TrainState* train_state, int dest, int dest_offset)
{
    p->state = train_state;
    int node = train_state->node;

    assert (node != dest);
    int* previous;
    int* distances;
    // track_node* track;

    switch(track_id)
    {
        case 'A':
            previous = shortest_paths_a[node];
            distances = shortest_distances_a[node];
            // track = tracka;
            break;

        case 'B':
            previous = shortest_paths_b[node];
            distances = shortest_distances_b[node];
            // track = trackb;
            break;

        default:
            assert(0);
            return; // To stop distances uninitialized error
            break;
    }
    
    // p->state.node = node;
    // p->state.offset = offset;
    
    assert(previous[0] != UNREACHABLE);

    // TODO Precompute the following?

    // Pull out and reverse paths from previous arrays created by dijkstras.
    int prev = dest;
    int total_distance = distances[dest];

    int path_nodes = 0;

    assert(previous[prev] != -1); // Creating path between nodes that are unreachable

    while(prev != -1)
    {
        p->path[path_nodes] = prev;
        prev = previous[prev];

        path_nodes++;
    }
    p->path[path_nodes] = node; // Put origin in the path

    p->path_len = path_nodes + 1;

    // Reverse the path list which are in reverse order
    for(int i = 0; i != p->path_len/2; i++) 
    {
        int temp = p->path[i];
        p->path[i] = p->path[p->path_len - 1 - i];
        p->path[p->path_len - 1 - i] = temp;
    }

    // Create distance array, invert distances to make them 
    // distance remaining instead of distance covered.
    for(int i = 0; i != p->path_len; i++)
        p->path_distance[i] = total_distance - distances[p->path[i]];

    p->current_node = 0;

    assert(p->state->node == p->path[p->current_node]);

    // Apply dest offsets to all distances, and remove last node if negative offset.
    for(int i = 0; i != p->path_len; i++) p->path_distance[i] += dest_offset;

    // Remove nodes in path left with negative distances (if offset was negative, some nodes may be rendered useless)
    while(p->path_len - 1 >= 0 && p->path_distance[ p->path_len - 1 ] < 0) p->path_len--;

    assert(p->path_len > 0);
}

TrainState create_train_state(int node, int offset, int speed)
{
    TrainState result;
    result.node = node;
    result.offset = offset;
    result.speed = speed;

    return result;
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

// In the future this will be an array of train_states
// Does not interpolate, simple snaps train into position based on sensor hits.
void update_train_states(TrainState* train_state, char* newly_triggered)
{
    int sensor_triggered = -1;
    // For now assume only one train can trigger.  In the future will use train states distances
    // from each sensor to figure out which train state to update.

    // Find newly triggered sensor
    for(int i = 0; i != MAX_SENSOR_NUMBER; i++)
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

// For now assume using the following train and the following start position 
#define CONTROLLED_TRAIN 1

#define ORIGIN "A5"
#define DESTINATION "E14"

void train_control_server(void)
{
    RegisterAs("train_control");

    tcid = WhoIs("tc_server");

    int cid = WhoIs("clock_server");
    int pid = WhoIsWait(cid, "terminal");

    Create(3, train_control_sensor_state_notifier);

    int sender;
    char msg[11];
    char reply_msg[1];

    // UNCOMMENT FOR STACK ALLOCATED SHORTEST PATH DATA
        // track_node _tracka[TRACK_MAX];
        // track_node _trackb[TRACK_MAX];

        // // For each node, a complete path 
        // int _shortest_paths_a[TRACK_MAX][TRACK_MAX];
        // int _shortest_paths_b[TRACK_MAX][TRACK_MAX];

        // // Distances for above paths
        // int _shortest_distances_a[TRACK_MAX][TRACK_MAX];
        // int _shortest_distances_b[TRACK_MAX][TRACK_MAX];

        // // Set globals for use in helper functions
        // // Note that these globals are not used by any other task than this one.
        // tracka = _tracka;
        // trackb = _trackb;
        // shortest_paths_a     = _shortest_paths_a;
        // shortest_paths_b     = _shortest_paths_b;
        // shortest_distances_a = _shortest_distances_a;
        // shortest_distances_b = _shortest_distances_b;

    track_constants = create_track_constants();

    // State variables
    track_id = 0;

    initialize();

    // TEMP hardcoded source and dest node indices
    int source = sensor_string_index(ORIGIN);
    int dest = sensor_string_index(DESTINATION);

    track = tracka;
    track_id = 'A';

    // Initialize single train state 
    TrainState train_state = create_train_state(source, 0, 0);

    TrainPathPlan plan;
    init_train_path_plan(&plan, &train_state, dest, 0);

    // Parsed sensor states 
    char sensor_states[MAX_SENSOR_NUMBER];
    char newly_triggered_sensors[MAX_SENSOR_NUMBER];

    // // Verify path is correct.
    // print("path length %d", plan.path_len);
    // for(int i = 0; i != plan.path_len; i++) 
    // {
    //     // print("type: %d\n\r", (int)track[plan.path[i]].type);
    //     if(track[plan.path[i]].type == NODE_SENSOR) print("%s ", track[plan.path[i]].name);
    // }
    int cnt = 0;
    for(;;)
    {
        // Receive clock server request
        Receive(&sender, msg, 11);

        // Execute the command encoded in the request
        switch(msg[0])
        {
            case SET_TRACK:
                track_id = msg[1];
                if(track_id == 'A') track = tracka;
                else if(track_id == 'B') track = trackb;
                Reply(sender, reply_msg, 0);
                break;

            case TARGET_POSITION:
                _target_position(msg[1], msg[2], unpack_int(msg + 3));
                Reply(sender, reply_msg, 0);
                break;
            case SET_POSITION:

                Reply(sender, reply_msg, 0);
                break;

            case SENSOR_DUMP:
                parse_sensors(msg + 1, sensor_states, newly_triggered_sensors);
                update_train_states(&train_state, newly_triggered_sensors);
                
                Reply(sender, reply_msg, 0);

                TPrintAt(pid, 0, 40, "train_location: %s   ", track[train_state.node].name);
                TPrintAt(pid, 0, 41, "cnt: %d", cnt++);
                break;
        }
    }
}
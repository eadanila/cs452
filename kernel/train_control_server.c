#include "train_control_server.h"
#include "tc_server.h"
#include "string_utility.h"
#include "syscall.h"
#include "name_server.h"
#include "track.h"
#include "logging.h"
#include "sensors.h"

#define TARGET_POSITION 1
#define SET_POSITION 2
#define GET_POSITION 3
#define SET_TRACK 4

#define INFINITY __INT_MAX__
#define NEIGHBOR_MAX 3

#define TRACK_A_SIZE 144
#define TRACK_B_SIZE 140

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

int GetPosition(int tid, char train_id, TrackPosition* tp)
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

int tcid;
track_node tracka[TRACK_MAX];
track_node trackb[TRACK_MAX];

// For each node, a complete path 
int shortest_paths_a[TRACK_MAX][TRACK_MAX];
int shortest_paths_b[TRACK_MAX][TRACK_MAX];

// Distances for above paths
int shortest_distances_a[TRACK_MAX][TRACK_MAX];
int shortest_distances_b[TRACK_MAX][TRACK_MAX];

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
                return track[u].edge[DIR_AHEAD].dist;

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
                return track[u].edge[DIR_STRAIGHT].dist;

            if(track + v == track[u].edge[DIR_CURVED].dest) 
                return track[u].edge[DIR_CURVED].dist;

            // if(track + v == track[u].reverse) 
            //     // The reverse track may be any type of track
            //     // reverse order of u, v and return that dist instead.
            //     link_cost(track, v, u);
            break;

        case NODE_NONE:
            assert(0);
            break;
    }

    return INFINITY;
}

void get_neighbors(track_node* track, int u, int* neighbors, int* size)
{
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
    // In this implementation, assume every node can be reached from every other node

    // Initialize distance and previous arrays
    for(int i = 0; i != TRACK_MAX; i++)
    {
        distance[i] = INFINITY;
        previous[i] = -1;
    } 

    int N[TRACK_MAX];
    N[0] = source;
    int N_size = 1;

    int neighbors[NEIGHBOR_MAX];
    int neighbors_size = 0;

    get_neighbors(track, source, neighbors, &neighbors_size);

    for(int i = 0; i != neighbors_size; i++) distance[neighbors[i]] = link_cost(track, source, neighbors[i]);

    for(;;)
    {
        int w = -1;
        int w_dist = INFINITY;

        // Find w not in N' with minimum D(u,w)
        for(int i = 0; i != track_node_count; i++)
        {
            if(!is_contained(i, N, N_size) && distance[i] < w_dist)
            {
                w = i;
                w_dist = distance[i];
            }
        }
        
        assert(w != -1);

        // Add w to N'
        N[N_size] = w;
        N_size++;

        int w_neighbors[NEIGHBOR_MAX];
        int w_neighbors_size;
        get_neighbors(track, w, w_neighbors, &w_neighbors_size);

        // For all v not in N' and adjacent to w
        for(int i = 0; i != w_neighbors_size; i++)
        {
            int v = w_neighbors[i];
            
            if(is_contained(v, N, N_size)) continue;

            if(distance[w] + link_cost(track, w, v) < distance[v])
            {
                distance[v] = distance[w] + link_cost(track, w, v);
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
    for(int i = 0; i != TRACK_A_SIZE; i++)
                // (track_node* track, const int track_node_count, int source, int* distance, int* previous)
        shortest_path(tracka, TRACK_A_SIZE, i, shortest_distances_a[i], shortest_paths_a[i]);

    for(int i = 0; i != TRACK_B_SIZE; i++)
        shortest_path(trackb, TRACK_B_SIZE, i, shortest_distances_b[i], shortest_paths_b[i]);
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
void init_train_path_plan(TrainPathPlan* p, int node, int offset, int dest, int dest_offset)
{
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
    
    p->pos.node = node;
    p->pos.offset = offset;
    
    assert(previous[0] != -1);

    // TODO Precompute the following?

    // Pull out and reverse paths from previous arrays created by dijkstras.
    int prev = dest;
    int total_distance = distances[dest];

    int path_nodes = 0;
    while(prev != node)
    {
        p->path[path_nodes] = prev;
        prev = previous[prev];

        path_nodes++;
    }
    p->path[path_nodes] = prev; // Put origin in the path

    p->path_len = path_nodes + 1;

    // Reverse the path list which are in reverse order
    for(int i = 0; i != p->path_len/2; i++) 
    {
        int temp = p->path[i];
        p->path[i] = p->path[p->path_len - 1 - i];
        p->path[p->path_len - 1 - i] = temp;
    }

    // Create distance array, invert distances to make them 
    // distance left instead of distance covered.
    for(int i = 0; i != p->path_len; i++)
        p->path_distance[i] = total_distance - distances[p->path[i]];

    p->current_node = 0;

    assert(p->pos.node == p->current_node);

    // Apply dest offsets to all distances, and remove last node if negative offset.
    for(int i = 0; i != p->path_len; i++) p->path_distance[i] += dest_offset;

    // Remove nodes in path left with negative distances (if offset was negative, some nodes may be rendered useless)
    while(p->path_len - 1 >=0 && p->path_distance[ p->path_len - 1 ] < 0) p->path_len--;

    assert(p->path_len > 0);
}

// For now assume using the following train and the following start position 
#define CONTROLLED_TRAIN 1

#define ORIGIN "A5"
#define DESTINATION "E14"

void train_control_server(void)
{
    RegisterAs("train_control");

    tcid = WhoIs("tc_server");

    int sender;
    char msg[9];
    char reply_msg[1];

    // State variables
    track_id = 0;

    initialize();

    // TEMP hardcoded source and dest node indices
    int source = sensor_string_index(ORIGIN);
    int dest = sensor_string_index(DESTINATION);

    TrainPathPlan plan;
    init_train_path_plan(&plan, source, 0, dest, 0);
    // _target_position(CONTROLLED_TRAIN, DESTINATION, 0);

    for(;;)
    {
        // Receive clock server request
        Receive(&sender, msg, 6);

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
        }
    }
}
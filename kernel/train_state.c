#include "train_state.h"
#include "shortest_path.h"
#include "logging.h"

// NOTE: Assuming dest_offset does not reach the node before or after it.
// TODO Add an error return for this ^
// A shortest path from node to dest must exist.
void init_train_path_plan(TrainPathPlan* p, TrainState* train_state, 
                          int* previous, int* distances,
                          int dest, int dest_offset, char end_speed)
{
    p->valid = 1;
    
    p->state = train_state;
    p->end_speed = end_speed;
    p->next_sensor_time = -1;

    int node = train_state->node;

    assert (node != dest);

    // p->state.node = node;
    // p->state.offset = offset;
    assert(previous[dest] != UNREACHABLE);
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

void init_train_state(TrainState* result, int id, int node, int offset, int speed)
{
    result->valid = 1;

    result->id = id;
    result->node = node;
    result->offset = offset;
    result->speed = speed;
}

int is_valid_speed(int speed_id)
{
    // TODO Needs to be updated
    return ((speed_id == TRAIN_SPEED_MAX) || (speed_id == TRAIN_SPEED_MIN));
}

int next_sensor(TrainPathPlan* plan, struct track_node* track)
{
    if(plan->current_node >= plan->path_len - 1) return -1; // No next node
    int current_node = plan->current_node + 1; // Skip the current node

    while(current_node < plan->path_len && track[plan->path[current_node]].type != NODE_SENSOR)
        current_node++;
    
    if(current_node == plan->path_len - 1) return -1; // No next sensor node

    assert(track[plan->path[current_node]].type == NODE_SENSOR);

    // Otherwise current node is a sensor node!
    return current_node;
}
#ifndef SHORTEST_PATH_H
#define SHORTEST_PATH_H

#include "track_node.h"
#include "track.h"

#define INFINITY 1000000 //CANNOT BE INT MAX SINCE IT NEEDS TO BE ADDED TO SAFELY
#define UNREACHABLE -1
#define NEIGHBOR_MAX 3

int link_cost(track_node* track, int u, int v);
void get_neighbors(track_node* track, int u, int* neighbors, int* size);

// distance and previous must both be TRACK_MAX size arrays
// NOTE: Assumes switches may be switched to obtain shortest path.
void shortest_path(track_node* track, const int track_node_count, int source, int* distance, int* previous);

#endif
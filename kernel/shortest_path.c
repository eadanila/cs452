#include "shortest_path.h"
#include "logging.h"

// Helpers for dijkstras
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
            break;

        case NODE_SENSOR:
        case NODE_MERGE:
        case NODE_ENTER: // 1 edge
            neighbors[0] = (int)(track[u].edge[DIR_AHEAD].dest - track);
            // neighbors[1] = (int)(track[u].reverse - track);
            *size += 1;
            break;

        case NODE_BRANCH: // 2 edges
            neighbors[0] = (int)(track[u].edge[DIR_STRAIGHT].dest - track); 
            neighbors[1] = (int)(track[u].edge[DIR_CURVED].dest - track);
            // neighbors[2] = (int)(track[u].reverse - track);
            *size += 2;
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
    int neighbors_size = -1;

    get_neighbors(track, source, neighbors, &neighbors_size);

    assert(neighbors_size != -1)

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
        
        // This means all w not in N' are unreachable, so we're finished.
        // if(w_dist == INFINITY) return;
        assert(w != UNREACHABLE);

        // Add w to N'
        N_visited[w] = 1;
        N_size++;

        int w_neighbors[NEIGHBOR_MAX];
        int w_neighbors_size = -1;
        get_neighbors(track, w, w_neighbors, &w_neighbors_size);
        assert(w_neighbors_size != -1);

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
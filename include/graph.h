#ifndef _GRAPH_H
#define _GRAPH_H

#define MAX            100000000
#define INT_MAX        100000000
#define BILLION 1E9

int create_weight_graph( int N, int DEG, int*** W_ret, int*** W_index_ret );
int create_distance_list(int N, int** D, int** Q);
int initialize_single_source(int* D, int* Q, int source, int N);
void init_weights(int N, int DEG, int** W, int** W_index);

#endif

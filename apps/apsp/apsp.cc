// vim: ts=3 sw=3 sts=0 smarttab expandtab
/*
  Program adopted from Parallel MiBench
*/

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <pthread.h>
//#include "carbon_user.h"  /*For the Graphite Simulator*/
#include <time.h>
#include <sys/timeb.h>
#include <iostream>
#include "roi.h"
#include "graph.h"

#define MAX            100000000
#define INT_MAX        100000000
#define BILLION 1E9

//Thread Argument Structure
typedef struct
{
   int*      local_min;
   int*      global_min;
   int*      Q;
   int*      D;
   int**     W;
   int**     W_index;
   int*      d_count;
   int       tid;
   int       P;
   int       N;
   int       DEG;
   pthread_barrier_t* barrier_total;
   pthread_barrier_t* barrier;
} thread_arg_t;


//Global Variables
int min = INT_MAX;    //For local mins
int min_index = 0;
pthread_mutex_t lock;
pthread_mutex_t locks[4194304];  //unused
int u = 0;                       //next best vertex
int local_min_buffer[1024];
int global_min_buffer;
int next_source = -1;
int start = 64;
int P_global = 256;
thread_arg_t thread_arg[1024];
pthread_t   thread_handle[1024];  //MAX threads and pthread handlers

//Primary Parallel Function
void* do_work(void* args)
{
   volatile thread_arg_t* arg = (thread_arg_t*) args;

   //volatile int* Q          = arg->Q;
   //int* D                   = arg->D;
   int** W                  = arg->W;       //Graph weights
   int** W_index            = arg->W_index; //Graph connections
   const int N              = arg->N;       //Total Vertices
   const int DEG            = arg->DEG;     //Edges per Vertex
   int v                    = 0;            //current vertex
   P_global                 = start;

   int node = 0;

   pthread_barrier_wait(arg->barrier_total);

   while(node<N)
   {
      pthread_mutex_lock(&lock);   //Vertex Capture
      next_source++;
      node = next_source;
      pthread_mutex_unlock(&lock);  

      if ( node >= N ) break;

      //Memory allocations
      int *D;
      int *Q;
      if (posix_memalign((void**) &D, 64, N * sizeof(int))) 
      {
         fprintf(stderr, "Allocation of memory failed\n");
         exit(EXIT_FAILURE);
      }
      if ( posix_memalign((void**) &Q, 64, N * sizeof(int)))
      {
         fprintf(stderr, "Allocation of memory failed\n");
         exit(EXIT_FAILURE);
      }
      //Initialize distance arrays
      initialize_single_source(D, Q, node, N);

      //Relax all edges, Bellman-Ford type
      for(v=0;v<N;v++)
      {
         for(int i = 0; i < DEG; i++)
         {
            if((D[W_index[v][i]] > (D[v] + W[v][i])))
               D[W_index[v][i]] = D[v] + W[v][i];

            Q[v]=0; //Current vertex checked
         }
      }
   }

   pthread_barrier_wait(arg->barrier_total);
   return NULL;
}

void usage(char **argv)
{
   std::cerr << "usage:" << std::endl;
   std::cerr << "\t" << argv[0] << " <threads> <N> <DEG>" << std::endl;
   exit(EXIT_FAILURE);
}

int main(int argc, char** argv)
{
   //Input arguments
   if ( argc < 4 ) usage(argv);

   const int P1 = atoi(argv[1]);
   const int N = atoi(argv[2]);
   const int DEG = atoi(argv[3]);

   int P = 256;
   start = P1;
   P = P1;

   if (DEG > N)
   {
      std::cerr << "Degree of graph cannot be grater than number of Vertices" << std::endl;
      exit(EXIT_FAILURE);
   }

   //int* D;
   //int* Q;
   //posix_memalign((void**) &D, 64, N * sizeof(int));
   //posix_memalign((void**) &Q, 64, N * sizeof(int));
   int d_count = N;
   pthread_barrier_t barrier_total;
   pthread_barrier_t barrier;

   int** W;
   int** W_index;
   if ( create_weight_graph( N, DEG, &W, &W_index) != 0 )
   {
      fprintf(stderr, "Could not allocate memory\n");
      exit(EXIT_FAILURE);
   }
   
   //Initialize random graph
   init_weights(N, DEG, W, W_index);

   //Synchronization Variables' Initialization
   pthread_barrier_init(&barrier_total, NULL, P1);
   pthread_barrier_init(&barrier, NULL, P1);
   pthread_mutex_init(&lock, NULL);
   for(int i=0; i<2097152; i++)
      pthread_mutex_init(&locks[i], NULL);

   //Thread Arguments
   for(int j = 0; j < P1; j++) {
      thread_arg[j].local_min  = local_min_buffer;
      thread_arg[j].global_min = &global_min_buffer;
      //thread_arg[j].Q          = Q;
      //thread_arg[j].D          = D;
      thread_arg[j].W          = W;
      thread_arg[j].W_index    = W_index;
      thread_arg[j].d_count    = &d_count;
      thread_arg[j].tid        = j;
      thread_arg[j].P          = P;
      thread_arg[j].N          = N;
      thread_arg[j].DEG        = DEG;
      thread_arg[j].barrier_total = &barrier_total;
      thread_arg[j].barrier    = &barrier;
   }

   roi_begin();

   //Spawn Threads
   for(int j = 1; j < P1; j++) {
      pthread_create(thread_handle+j,
            NULL,
            do_work,
            (void*)&thread_arg[j]);
   }
   do_work((void*) &thread_arg[0]);

   //Join Threads
   for(int j = 1; j < P1; j++) 
   {
      pthread_join(thread_handle[j],NULL);
   }

   roi_end();

   return 0;
}


// vim: ts=3 sw=3 sts=0 smarttab expandtab

#include <cstdlib>
#include <pthread.h>
#include <iostream>

#include "graph.h"

int create_weight_graph( int N, int DEG, int *** W_ret, int *** W_index_ret )
{
   std::cout << "Creating graph with " << N << " verticies with degree " << DEG << std::endl;

   //Memory allocations for the input graph
   int** W = (int**) malloc(N*sizeof(int*));
   int** W_index = (int**) malloc(N*sizeof(int*));
   for(int i = 0; i < N; i++)
   {
      int ret;
      ret = posix_memalign((void**) &W[i], 64, DEG*sizeof(int));
      if ( ret != 0 ) return ret;

      ret = posix_memalign((void**) &W_index[i], 64, DEG*sizeof(int));
      if ( ret != 0 ) return ret;
   }
   *W_ret = W;
   *W_index_ret = W_index;

   return 0;
}

int create_distance_list(int N, int** D, int** Q)
{
   int ret;
   ret = posix_memalign((void**) D, 64, N * sizeof(int));
   if ( ret != 0 ) return ret;

   ret = posix_memalign((void**) Q, 64, N * sizeof(int));
   if ( ret != 0 ) return ret;

   return 0;
}

//Distance initializations
int initialize_single_source(int*  D,
      int*  Q,
      int   source,
      int   N)
{
   for(int i = 0; i < N; i++)
   {
      D[i] = INT_MAX;  //all distances infinite
      Q[i] = 1;
   }

   D[source] = 0;      //source distance 0
   return 0;
}

//Graph initializer
void init_weights(int N, int DEG, int** W, int** W_index)
{
   // Initialize to -1
   for(int i = 0; i < N; i++)
      for(int j = 0; j < DEG; j++)
         W_index[i][j]= -1;

   // Populate Index Array
   for(int i = 0; i < N; i++)
   {
      int last = 0;
      for(int j = 0; j < DEG; j++)
      {
         if(W_index[i][j] == -1)
         {
            int neighbor = i + j;
            if(neighbor > last)
            {
               W_index[i][j] = neighbor;
               last = W_index[i][j];
            }
            else
            {
               if(last < (N-1))
               {
                  W_index[i][j] = (last + 1);
                  last = W_index[i][j];
               }
            }
         }
         else
         {
            last = W_index[i][j];
         }
         if(W_index[i][j]>=N)
         {
            W_index[i][j] = N-1;
         }
      }
   }

   // Populate Cost Array
   for(int i = 0; i < N; i++)
   {
      for(int j = 0; j < DEG; j++)
      {
         double v = drand48();
         if(W_index[i][j] == i)
            W[i][j] = 0;
         else
            W[i][j] = (int) (v*100) + 1;
      }
   }
}

// vim: ts=3 sw=3 sts=0 smarttab expandtab
/*
    Distributed Under the MIT license
	Uses vertex coloring to distinguish searches
    Programs by Masab Ahmad (UConn)
*/

#include <cstdio>
#include <cstdlib>
#include <pthread.h>
//#include "carbon_user.h"     /* For the Graphite Simulator*/
#include <time.h>
#include <sys/timeb.h>
#include "roi.h"
#include "graph.h"

//Thread Argument Structure
typedef struct
{
   int*      local_min;
   int*      global_min;
   int*      Q;
   int*      D;
   //int**     W;
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
pthread_mutex_t lock;            //single lock
pthread_mutex_t locks[4194304];  //locks for each vertes, upper limit
int local_min_buffer[1024];
int global_min_buffer;
int Total = 0;
int terminate = 0;               //work termination
int P_global = 256;
int *edges;                       //deg of a given vertex
int *exist;                      //whether vertex in graph
int *temporary;
int largest=0;
thread_arg_t thread_arg[1024];
pthread_t   thread_handle[1024];

//Primary Parallel Function
void* do_work(void* args)
{
   //Thread variables and arguments
   volatile thread_arg_t* arg = (thread_arg_t*) args;
   int tid                  = arg->tid;  //thread id
   int P                    = arg->P;    //Max threads
   volatile int* Q          = arg->Q;    //set/unset array
   int* D                   = arg->D;    //coloring array
   //int** W                  = arg->W;
   int** W_index            = arg->W_index;  //graph structure
   int v = 0;
   int iter = 0;

   //For precision work allocation
   double P_d = P;
   double tid_d = tid;
   double largest_d = largest+1.0;

   int start =  0;  //tid    * DEG / (arg->P);
   int stop  = 0;   //(tid+1) * DEG / (arg->P);

   //Partition data into threads
   double start_d = (tid_d) * (largest_d/P_d);
   double stop_d = (tid_d+1.0) * (largest_d/P_d);
   start = start_d; //tid    *  (largest+1) / (P);
   stop = stop_d; //(tid+1) *  (largest+1) / (P);

   //printf("\n tid:%d %d %d",tid,start,stop);

   pthread_barrier_wait(arg->barrier_total);

   while(terminate==0)
   {   
      for(v=start;v<stop;v++)
      {
         if(exist[v]==0)
            continue;                              //if not in graph
         //printf("\nv:%d Q:%d %d",v, Q[v], D[v]);
         if(D[v]==0 || D[v]==2)                    //already colored
            continue;
         //printf("\nuu:%d Q:%d %d",v, Q[v], P); 
         //D[v]=2;

         for(int i = 0; i < edges[v]; i++)
         {   
            int neighbor = W_index[v][i];
            pthread_mutex_lock(&locks[neighbor]);
            if(Q[neighbor]==1)                       //if unset then set
               Q[neighbor]=0;                        //Can be set to Parent
            temporary[neighbor] = 1;
            pthread_mutex_unlock(&locks[neighbor]);
         }
      }
      //if(tid==0) printf("\n %d",Q[largest]);

      pthread_barrier_wait(arg->barrier_total);
    
      //Update colors	
      for(v=start;v<stop;v++)
      {
         D[v] = temporary[v];
      }
 
      //Termination Condition
      if(Q[largest]==0 || iter>=Total)
        terminate=1;
      iter++;
      pthread_barrier_wait(arg->barrier_total);
   }
   //printf("\n %d %d",tid,terminate);
   pthread_barrier_wait(arg->barrier_total);

   return NULL;
}

//Main
int main(int argc, char** argv)
{
   FILE *file0 = NULL;
   int N = 0;
   int DEG = 0;
   //whether read from file or generate synthetic
   const int select = atoi(argv[1]);

   //if reading from file
   if(select==1)
   {
      const char *filename = argv[3];
      //printf("Please Enter The Name Of The File You Would Like To Fetch\n");
      //scanf("%s", filename);
      file0 = fopen(filename,"r");
   }

   int lines_to_check=0;
   char c;
   int number0;
   int number1;
   int inter = -1; 

   if(select==1)
   {
      N = 2097152; //can be read from file if needed, this is a default upper limit
      DEG = 16;     //also can be reda from file if needed, upper limit here again
   }

   //Max input threads
   const int P1 = atoi(argv[2]);

   int P = P1;
   P_global = P1;

   //If generating a uniform random graph
   if(select==0)
   {
      N = atoi(argv[3]);
      DEG = atoi(argv[4]);
      printf("\nGraph with Parameters: N:%d DEG:%d\n",N,DEG);
   }

   if (DEG > N)
   {
      fprintf(stderr, "Degree of graph cannot be grater than number of Vertices\n");
      exit(EXIT_FAILURE);
   }

   //Memory Allocations
   int* D;
   int* Q;
   if(posix_memalign((void**) &D, 64, N * sizeof(int)))
   {
      fprintf(stderr, "Allocation of memory failed\n");
      exit(EXIT_FAILURE);
   }
   if(posix_memalign((void**) &Q, 64, N * sizeof(int)))
   {
      fprintf(stderr, "Allocation of memory failed\n");
      exit(EXIT_FAILURE);
   }
   if(posix_memalign((void**) &edges, 64, N * sizeof(int)))
   {
      fprintf(stderr, "Allocation of memory failed\n");
      exit(EXIT_FAILURE);
   }
   if(posix_memalign((void**) &exist, 64, N * sizeof(int)))
   {
      fprintf(stderr, "Allocation of memory failed\n");
      exit(EXIT_FAILURE);
   }
   if(posix_memalign((void**) &temporary, 64, N * sizeof(int)))
   {
      fprintf(stderr, "Allocation of memory failed\n");
      exit(EXIT_FAILURE);
   }
   int d_count = N;
   pthread_barrier_t barrier_total;
   pthread_barrier_t barrier;

   int** W = (int**) malloc(N*sizeof(int*));
   int** W_index = (int**) malloc(N*sizeof(int*));
   if ( create_weight_graph( N, DEG, &W, &W_index) != 0 )
   {
      fprintf(stderr, "Could not allocate memory\n");
      exit(EXIT_FAILURE);
   }

   //Memory initialization
   for(int i=0;i<N;i++)
   {
      for(int j=0;j<DEG;j++)
      {
         //W[i][j] = 1000000000;
         W_index[i][j] = INT_MAX;
      }
      edges[i]=0;
      exist[i]=0;
      temporary[i]=0;
   }

   //If reading from file
   if(select==1)
   {
      for(c=getc(file0); c!=EOF; c=getc(file0))
      {
         if(c=='\n')
            lines_to_check++;

         if(lines_to_check>3)
         {
            int f0 = fscanf(file0, "%d %d", &number0,&number1);
            if(f0 != 2 && f0 != EOF)
            {
               printf ("Error: Read %d values, expected 2. Parsing failed.\n",f0);
               exit (EXIT_FAILURE);
            }
            //printf("\n%d %d",number0,number1);
            if(number0>largest)
               largest=number0;
            if(number1>largest)
               largest=number1;
            inter = edges[number0];

            //W[number0][inter] = drand48();
            W_index[number0][inter] = number1;
            //previous_node = number0;
            edges[number0]++;
            exist[number0]=1; exist[number1]=1;
         }
      }
      //printf("\n%d deg:%d",test[0]);
      printf("\nFile Read, Largest Vertex:%d",largest);
   }

   //Generate Random graph
   if(select==0)
   {
      init_weights(N, DEG, W, W_index);
      largest = N-1; //largest vertex id
   }

   //Synchronization variables
   pthread_barrier_init(&barrier_total, NULL, P);
   pthread_barrier_init(&barrier, NULL, P);

   pthread_mutex_init(&lock, NULL);

   for(int i=0; i<largest+1; i++)
   {
      if(select==0)
      {
         exist[i] = 1;
         edges[i] = DEG;
      }
      if(exist[i]==1)
      {
         Total++;
         pthread_mutex_init(&locks[i], NULL);
      }
   }
   //printf("\n %d %d %d",N,largest,Total);

   //Initialize Data Structures
   initialize_single_source(D, Q, 0, N);

   //Thread arguments
   for(int j = 0; j < P; j++) {
      thread_arg[j].local_min  = local_min_buffer;
      thread_arg[j].global_min = &global_min_buffer;
      thread_arg[j].Q          = Q;
      thread_arg[j].D          = D;
      //thread_arg[j].W          = W;
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
   for(int j = 1; j < P; j++) {
      pthread_create(thread_handle+j,
            NULL,
            do_work,
            (void*)&thread_arg[j]);
   }
   do_work((void*) &thread_arg[0]);  //master thread initializes itself

   //Join threads
   for(int j = 1; j < P; j++) { //mul = mul*2;
      pthread_join(thread_handle[j],NULL);
   }

   roi_end();

   return 0;
}


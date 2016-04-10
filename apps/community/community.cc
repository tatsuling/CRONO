#include <cstdio>
#include <cstdlib>
#include <pthread.h>
//#include "carbon_user.h"       /*For the Graphite Simulator*/
#include <time.h>
#include <sys/timeb.h>
#include "roi.h"

#define MAX            100000000
#define INT_MAX        100000000
#define BILLION 1E9

//Thread Argument Structure
typedef struct
{
   float*      Q;
   int*        D;
   float**     W;                    //Graph edge weights
   int**       W_index;              //Graph edge connections
   float*      mod_gain;
   float*      total_mod_gain;
   int*        comm;                 //Community data structure
   float*      C;
   int*        d_count;
   int         tid;                  //Thread ID
   int         P;                    //MAX threads
   int         N;                    //Total vertices
   int         DEG;                  //Edges per vertex
   pthread_barrier_t* barrier_total;
   pthread_barrier_t* barrier;
} thread_arg_t;

//Function declarations
int initialize_single_source(int* D, float* Q, int source, int N);
void init_weights(int N, int DEG, float** W, int** W_index);

//Global Variables
pthread_mutex_t lock;                          //single lock
pthread_mutex_t locks[2097152];                //Max locks for each vertex
int iterations = 0;                            //Iterations for community over the graph
int *edges;                                    //Edges per vertex
int *exist;                                    //If vertex exists in the graph
int largest=0;                                 //Final vertex
int P_global = 256;
thread_arg_t thread_arg[1024];
pthread_t   thread_handle[1024];               //MAX threads and pthread handlers
int v_test = -1;

//Primary Parallel Function
void* do_work(void* args)
{
   volatile thread_arg_t* arg = (thread_arg_t*) args;

   int tid                  = arg->tid;
   int P                    = arg->P;
   float** W                = arg->W;
   int** W_index            = arg->W_index;
   float* mod_gain          = arg->mod_gain;
   int* comm                = arg->comm;
   int local_count          = 0;
   int v                    = 0;                    //current vertex
   int i;
   int index                = 0;                    //stores edge id
   float sum_tot            = 0;
   float sum_in             = 0;
   double P_d = P;
   double tid_d = tid;
   double largest_d = largest+1.0;

   //float total_edges = N*DEG;
   float mod_gain_temp = 0;                         //temporary modularity gain for outer loop
   float mod_gain_temp_temp = 0;                    //temporary modularity gain for inner loop
   //float inv_total_edges = 2/total_edges;

   int start =  0;  //tid    * DEG / (arg->P);
   int stop  = 0;   //(tid+1) * DEG / (arg->P);

   //Allocate work via double precision
   double start_d = (tid_d) * (largest_d/P_d);
   double stop_d = (tid_d+1.0) * (largest_d/P_d);

   start = start_d; //tid    *  (largest+1) / (P);
   stop =  stop_d;//(tid+1) *  (largest+1) / (P);
   //printf("\n %d %d %d",tid, start,stop);
   //put each node in its own community

   pthread_barrier_wait(arg->barrier_total);

   //Each vertex is in it's own community
   for(v=start;v<stop;v++)
   {
      //for(int i=0;i<edges[v];i++)
      //{
      comm[v] = v;
      //}
   }

   v = 0;             //reset current vertex
   v_test = 0;

   pthread_barrier_wait(arg->barrier_total);

   //static for now
   //can be done more for dynamic, need a while loop here for thay
   //initialization of communities
   while(local_count<iterations)	
   {
      for(v=start;v<stop;v++)
         //while(v<N-2)
      {
         //pthread_mutex_lock(&lock);
         //v_test++;
         //v = v_test;
         //printf(" %d ",v);
         //pthread_mutex_unlock(&lock);
         if(exist[v]==0)
            continue;	
         for(i = 0; i < edges[v]; i++)
         {
            //pthread_mutex_lock(&locks[neighbor]);
            float total_edges = largest*edges[v];
            float inv_total_edges = 2/total_edges;
            int tempo = (inv_total_edges)*(inv_total_edges);
            tempo = tempo*2;
            int subtr = sum_tot*W[v][i];
            subtr = subtr*tempo;
            subtr = W[v][i] - subtr;
            mod_gain_temp_temp = (inv_total_edges)*(subtr);
            //total_mod_gain[v] = total_mod_gain[v] + mod_gain[v];

            if(mod_gain_temp_temp>mod_gain_temp) 
            {
               mod_gain_temp = mod_gain_temp_temp;
               index = W_index[v][i];
            }	

            //pthread_mutex_unlock(&locks[neighbor]);
         }
         mod_gain[v] = mod_gain_temp;
         comm[v] = index;   //cvk


         //update individual  sums
         //pthread_mutex_lock(&lock);
         sum_tot = sum_tot + W[v][i];
         sum_in = sum_in + W[v][i];
         //pthread_mutex_unlock(&lock);	
      }

      pthread_barrier_wait(arg->barrier_total);

      //reconstruct
      for(v=start;v<stop;v++)
      {
         if(exist[v]==0)
            continue;
         for(i=0;i<edges[v];i++)
         {
            int neighbor = W_index[v][i];
            pthread_mutex_lock(&locks[neighbor]);
            //W_index[v][i] = comm[neighbor];
            W[v][i] = comm[v] - comm[neighbor];
            pthread_mutex_unlock(&locks[neighbor]);
         }
      }

      pthread_barrier_wait(arg->barrier_total);
      local_count++;
   }
   //reduction heuristic approximate model, not very accurate though
   //if(tid==0)
   //{
   for(i=stop;i<largest+1;i++)
   {
      if(exist[v]==0)
         continue;
      //for(int j=1;j<P;j++)
      //{
      comm[i] = comm[((1) *  (largest+1) / (P))-1];
      //}
   }
   //}

   pthread_barrier_wait(arg->barrier_total);

   return NULL;
}

//Main
int main(int argc, char** argv)
{
   int N = 0;
   int DEG = 0;
   FILE *file0 = NULL;
   const int select = atoi(argv[1]);
   const int P1 = atoi(argv[2]);
   iterations = atoi(argv[3]);

   //If graph to be read from file
   if(select==1)
   {
      const char *filename = argv[4];
      file0 = fopen(filename,"r");
   }

   int lines_to_check=0;   //File reading variables
   char c;
   int number0;
   int number1;
   int inter = -1; 

   if(select==1)
   {
      N = 2097152; //can be read from file if needed, this is a default upper limit
      DEG = 16;     //also can be reda from file if needed, upper limit here again
   }

   int P = P1;
   P_global = P1;

   //If synthetic graph is to be generated
   if(select==0)
   {
      N = atoi(argv[4]);
      DEG = atoi(argv[5]);
      printf("\nGraph with Parameters: N:%d DEG:%d\n",N,DEG);
   }

   if (DEG > N)
   {
      fprintf(stderr, "Degree of graph cannot be grater than number of Vertices\n");
      exit(EXIT_FAILURE);
   }

   //Memory Allocations
   int* D;
   float* C;
   float* Q;
   int* comm;
   float* mod_gain;
   float* total_mod_gain;
   int* deg_node;
   if(posix_memalign((void**) &D, 64, N * sizeof(int)))
   {
      fprintf(stderr, "Allocation of memory failed\n");
      exit(EXIT_FAILURE);
   }
   if(posix_memalign((void**) &Q, 64, N * sizeof(float)))
   {
      fprintf(stderr, "Allocation of memory failed\n");
      exit(EXIT_FAILURE);
   }
   if(posix_memalign((void**) &deg_node, 64, N * sizeof(int)))
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

   if(posix_memalign((void**) &comm, 64, N * sizeof(int)))
   {
      fprintf(stderr, "Allocation of memory failed\n");
      exit(EXIT_FAILURE);
   }
   if(posix_memalign((void**) &C, 64, N * sizeof(float)))
   {
      fprintf(stderr, "Allocation of memory failed\n");
      exit(EXIT_FAILURE);
   }
   if(posix_memalign((void**) &mod_gain, 64, N * sizeof(float)))
   {
      fprintf(stderr, "Allocation of memory failed\n");
      exit(EXIT_FAILURE);
   }
   if(posix_memalign((void**) &total_mod_gain, 64, N * sizeof(float)))
   {
      fprintf(stderr, "Allocation of memory failed\n");
      exit(EXIT_FAILURE);
   }
   int d_count = N;

   float** W = (float**) malloc(N*sizeof(float*));
   int** W_index = (int**) malloc(N*sizeof(int*));
   for(int i = 0; i < N; i++)
   {
      //W[i] = (int *)malloc(sizeof(int)*N);
      if(posix_memalign((void**) &W[i], 64, DEG*sizeof(float)))
      {
         fprintf(stderr, "Allocation of memory failed\n");
         exit(EXIT_FAILURE);
      }
      if(posix_memalign((void**) &W_index[i], 64, DEG*sizeof(int)))
      {
         fprintf(stderr, "Allocation of memory failed\n");
         exit(EXIT_FAILURE);
      }
   } printf("\nMemory Allocated");

   //Memory initializations
   for(int i=0;i<N;i++)
   {
      for(int j=0;j<DEG;j++)
      {
         //W[i][j] = 1000000000;
         W_index[i][j] = INT_MAX;
         double v = drand48();
         if(W_index[i][j] == i)
            W[i][j] = 0;
         else
            W[i][j] = (int) (v*100) +1;
      }
      edges[i]=0;
      exist[i]=0;
   }

   //Read graph from file
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
      printf("\nFile Read, Largest Vertex:%d",largest);
   }

   pthread_barrier_t barrier_total;
   pthread_barrier_t barrier;

   //Generate synthetic graphs
   if(select==0)
   {
      init_weights(N,DEG,W,W_index);
      largest = N-1;
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
         pthread_mutex_init(&locks[i], NULL);
   }

   //Initialize data structures
   initialize_single_source(D, Q, 0, N);

   //Thread arguments
   for(int j = 0; j < P; j++) {
      thread_arg[j].Q          = Q;
      thread_arg[j].D          = D;
      thread_arg[j].W          = W;
      thread_arg[j].W_index    = W_index;
      thread_arg[j].comm       = comm;
      thread_arg[j].C          = C;
      thread_arg[j].mod_gain   = mod_gain;
      thread_arg[j].total_mod_gain = total_mod_gain;
      thread_arg[j].d_count    = &d_count;
      thread_arg[j].tid        = j;
      thread_arg[j].P          = P;
      thread_arg[j].N          = N;
      thread_arg[j].DEG        = DEG;
      thread_arg[j].barrier_total = &barrier_total;
      thread_arg[j].barrier    = &barrier;
   }
   
   //CPU clock
   struct timespec requestStart, requestEnd;
   clock_gettime(CLOCK_REALTIME, &requestStart);

   roi_begin();

   // Enable Graphite performance and energy models
   //CarbonEnableModels();

   //Spawn threads
   for(int j = 1; j < P; j++) {
      pthread_create(thread_handle+j,
            NULL,
            do_work,
            (void*)&thread_arg[j]);
   }
   do_work((void*) &thread_arg[0]);  //main thread spawns itself

   printf("\nThreads Returned!");

   //Join threads
   for(int j = 1; j < P; j++) { //mul = mul*2;
      pthread_join(thread_handle[j],NULL);
   }

   roi_end();

   // Disable Graphite performance and energy models
   //CarbonDisableModels();

   clock_gettime(CLOCK_REALTIME, &requestEnd);
   double accum = ( requestEnd.tv_sec - requestStart.tv_sec ) + ( requestEnd.tv_nsec - requestStart.tv_nsec ) / BILLION;
   printf( "\nTime:%lf seconds\n", accum );

   //printf("\ndistance:%d \n",D[N-1]);

   //Print Results
   /*for(int i = 0; i < largest+1; i++) {
     if(exist[i]==1)
     printf("\n %d %d ", i,comm[i]);
     }
     printf("\n");*/

   return 0;
}

int initialize_single_source(int*  D,
      float*  Q,
      int   source,
      int   N)
{
   for(int i = 0; i < N+1; i++)
   {
      D[i] = 0;
      Q[i] = 0;
   }

   return 0;
}

void init_weights(int N, int DEG, float** W, int** W_index)
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
            int neighbor = i+j;
            //W_index[i][j] = i+j;//rand()%(DEG);

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
         /*if(v > 0.8 || W_index[i][j] == -1)
           {       W[i][j] = MAX;
           W_index[i][j] = -1;
           }

           else*/ if(W_index[i][j] == i)
         W[i][j] = 0;

         else
            W[i][j] = (int) (v*100) + 1;
         //printf("   %d  ",W_index[i][j]);
      }
      //printf("\n");
   }
}

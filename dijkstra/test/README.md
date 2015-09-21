Parallel Dijkstra
=================

Synthetic Graphs : dijk_synthetic.cc
Real World File Input Graphs : dijk_real.cc

You can run ```make``` to create executables for each program or use the following commands below

**Synthetic Graphs**

To compile range_dijk.cc
  ```g++ dijk_synthetic.cc -o dijk_synthetic -lm -lrt -lpthread -O2```
  
To run with P number of threads, N vertices, and DEG edges per vertex
  ```./dijk_synthetic P N DEG```

e.g. ```./dijk_synthetic 2 16384 16```

**Real Graphs**

To compile dijk_real.cc
  ```g++ dijk_real.cc -o dijk_real -lm -lrt -lpthread -O2```
  
To run with P number of threads
  ```./dijk_real P```
  
  It will then ask for the input file, enter:
  sample.txt
  OR any other file such as road networks from the SNAP datasets (e.g. roadNet-CA)
  https://snap.stanford.edu/data/#road

**Notes**

The executable then outputs the time in seconds that the program took to run.
This parallelization can handle negative edge weights as well.
The real world graphs version has a parameter P_max, that is specified by ```cuberoot(N)*3```, which specifies the number of iterations for the outer loop.
Unlike other Graph Processing Frameworks like Ligra and Boost, we do not pre-process graphs (e.g. compression, vertex/edge reordering), and hence have a more generic implementation.

The test folder contains a testing program to check for inaccurate results.

If you use these programs please cite:

Masab Ahmad (UConn), Kartik Lakhsminarasimhan (UConn), Omer Khan (UConn), Efficient Parallelization of Path Planning Workload on Single-chip Shared-memory Multicores, In Proceedings of the IEEE High Performance Extreme Computing Conference (HPEC), Sept. 2015.

CRONO v0.9 : A Benchmark Suite for Multithreaded Graph Algorithms Executing on Futuristic Multicores
====================================================================

[![build status](https://travis-ci.org/masabahmad/CRONO.svg?branch=master)](https://travis-ci.org/masabahmad/CRONO)

This is a pre-release repository containing 10 graph analytic benchmarks.
An in-built synthetic graph generater can be included by statically configuring certain benchmarks.
These benchmarks can run on real multicore machines as well as the Graphite Multicore Simulator.

If you use these benchmarks, please cite:

CRONO : A Benchmark Suite for Multithreaded Graph Algorithms Executing on Futuristic Multicores, Masab Ahmad, Farrukh Hijaz, Qingchuan Shi, Omer Khan, IEEE International Symposium on Workload Characteriz
ation (IISWC), Oct 2015, Atlanta, Georgia, USA.

Paper pdf is located at: 
http://www.engr.uconn.edu/~omer.khan/pubs/crono-iiswc15.pdf

Requirements
============

1. Linux (Tested on Ubuntu 14.04)
2. g++ 4.6 (Tested with g++ 4.7)
3. The ```pthread``` Library

Features
========
1. Ubiquitous graph based search, planning, and clustering algorithms
2. Benchmarks use adjacency list representation for input graphs
3. Most benchmarks scale to 256 threads, some scale upto 1024 threads as well
4. Easy to compile and use
5. The input graphs are generic, i.e., the benchmarks do not assume any pre-processing or optimizations, such as graph compression and vertex/edge reordering

Getting Started
===============

Checkout the Repo:
```git clone https://github.com/masabahmad/CRONO```

To generate the executable for a benchmark, run ```make``` inside the CRONO directory, then execute each benchmark using the syntax specified by the individual README.md.

Contact
=======

masab.ahmad@uconn.edu

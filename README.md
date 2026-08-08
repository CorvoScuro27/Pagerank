# PageRank in C

A multithreaded implementation of the **PageRank algorithm** written in C using POSIX threads, semaphores, mutexes, and condition variables.

The project focuses not only on implementing the PageRank algorithm itself, but also on exploiting parallelism during both graph construction and PageRank computation.

## Overview

PageRank is an algorithm originally developed by Larry Page and Sergey Brin for ranking web pages according to the structure of the links between them.

The basic idea is that a node is considered important when it is referenced by other important nodes. The algorithm can be applied to any directed graph, where nodes represent entities and directed edges represent relationships between them.

This implementation represents the graph using:

* the number of outgoing edges for each node;
* an incoming-edge list for every node;
* dynamic memory allocation for the graph structure.

The PageRank computation is performed in parallel using multiple POSIX threads.

## Features

* PageRank computation using the iterative power method.
* Multithreaded PageRank computation with `pthread`.
* Parallel graph construction using a producer-consumer architecture.
* Bounded shared buffer for reading graph edges.
* Synchronization through POSIX semaphores and mutexes.
* Custom reusable barrier implemented using a mutex and a condition variable.
* Handling of **dead-end nodes** (nodes with no outgoing edges).
* Detection and removal of duplicate edges.
* Dynamic graph representation with memory trimming after construction.
* Configurable damping factor, convergence threshold, maximum number of iterations, number of threads, and number of top-ranked nodes.
* Support for Matrix Market-style input files, including comment lines beginning with `%`.
* Reporting of the top-`K` nodes according to their PageRank score.

## PageRank

The implementation uses the iterative formulation of PageRank.

For a graph with `N` nodes, the initial rank is uniformly distributed:

```text
x[i] = 1 / N
```

At each iteration, the rank of every node is updated according to its incoming links, the damping factor, and the contribution of dead-end nodes.

The implementation uses a damping factor `d` and computes:

```text
first_term = (1 - d) / N
```

The contribution of dead-end nodes is computed separately and distributed uniformly across all nodes.

The algorithm iterates until either:

1. the total error between two consecutive rank vectors becomes smaller than the specified tolerance; or
2. the maximum number of iterations is reached.

The error is computed as the sum of the absolute differences between consecutive PageRank vectors.

## Parallelization

The project uses POSIX threads at two different stages.

### 1. Graph construction

The input graph is read by a single producer thread and its edges are placed into a bounded circular buffer.

Multiple consumer threads retrieve edges from the buffer and insert them into the graph.

The producer-consumer communication is synchronized using:

* a semaphore counting free buffer slots;
* a semaphore counting available data items;
* a mutex protecting the shared buffer and consumer index.

A buffer size of `20` is currently used.

The consumer threads also:

* ignore self-loops;
* detect duplicate edges;
* update the graph structure.

This allows graph construction to be parallelized while keeping access to the shared data structure synchronized.

### 2. PageRank computation

The PageRank vector is divided into contiguous ranges, with each worker thread responsible for a portion of the nodes.

For every iteration, the threads synchronize at several points using a custom barrier.

The computation is divided into several stages:

1. Compute the contribution of dead-end nodes.
2. Compute the intermediate vector `y`.
3. Compute the next PageRank vector.
4. Compute the local error.
5. Combine the errors produced by all threads.
6. Check convergence.
7. Update the PageRank vector.

The barrier ensures that no thread starts a new stage before all other threads have completed the previous one.

## Synchronization

A reusable barrier is implemented using:

* `pthread_mutex_t`
* `pthread_cond_t`
* a thread counter
* a cycle counter

The cycle counter allows the same barrier to be reused across multiple PageRank iterations.

When the last thread reaches the barrier, it increments the cycle counter, resets the number of waiting threads, and wakes the other threads through a condition-variable broadcast.

## Graph Representation

The graph is represented by the following structure:

```c
typedef struct
{
    int N;
    int *out;
    inmap *in;
} grafo;
```

For every node:

* `out[i]` stores the number of outgoing edges;
* `in[i]` stores the list of nodes having an edge pointing to `i`.

Incoming edges are stored dynamically. The allocated capacity is initially set to `10` elements and is doubled whenever the current capacity is reached.

After the entire graph has been constructed, the allocated arrays are trimmed to their actual size to avoid unnecessary memory usage.

Duplicate edges are explicitly detected and ignored.

## Input Format

The program expects a graph file containing edges represented by pairs of node identifiers.

The input files in the `samples/` directory use the Matrix Market format. Comment lines beginning with `%` are ignored, and the first non-comment line containing three integers is used to determine the number of nodes and graph dimensions.

Example:

```text
% Matrix Market graph
9 9 18
1 2
1 3
2 4
...
```

The graph nodes in the input are numbered starting from `1`. Internally, node identifiers are converted to zero-based indexing.

The repository contains several sample graphs:

```text
samples/
├── 9nodi.mtx
├── 21archi.mtx
├── web-cnr.mtx
└── web-cnr.sol
```

## Building

The project requires a C compiler with support for C11 and POSIX threads.

The provided `makefile` uses `gcc` and compiles the project with:

```text
-std=c11 -Wall -g -O -pthread
```

The required libraries include:

```text
-lm -lrt -pthread
```

To build the project:

```bash
make
```

This produces the executable:

```text
pagerank
```

To remove the generated executable and object files:

```bash
make clean
```

The build process compiles the source files into the `obj/` directory before linking them into the final executable.

## Usage

The executable can be invoked as follows:

```text
./pagerank [-k K] [-m M] [-d D] [-e E] [-t T] infile
```

where:

| Option   | Description                           | Default  |
| -------- | ------------------------------------- | -------- |
| `-k K`   | Number of top-ranked nodes to display | `3`      |
| `-m M`   | Maximum number of PageRank iterations | `100`    |
| `-d D`   | Damping factor                        | `0.9`    |
| `-e E`   | Convergence tolerance                 | `1.0e-7` |
| `-t T`   | Number of worker threads              | `3`      |
| `infile` | Input graph file                      | Required |

These defaults are defined in `xerrori.h`, while command-line arguments are parsed using `getopt`.

### Example

To run PageRank on the sample graph using 4 threads:

```bash
./pagerank -t 4 samples/9nodi.mtx
```

A more customized execution could be:

```bash
./pagerank -k 5 -m 200 -d 0.85 -e 1e-8 -t 4 samples/web-cnr.mtx
```

## Output

The program reports information about the input graph and the PageRank computation.

Example output format:

```text
Number of nodes: ...
Number of dead-end nodes: ...
Number of valid arcs: ...
Converged after ... iterations
Sum of ranks: 1.0000 (should be 1)
Top K nodes:
 ...
```

The program checks that the sum of the resulting PageRank values is approximately `1` and reports the nodes with the highest scores.

The top-`K` nodes are obtained by sorting the PageRank values in descending order.

## Project Structure

```text
Pagerank/
├── samples/
│   ├── 9nodi.mtx
│   ├── 21archi.mtx
│   ├── web-cnr.mtx
│   └── web-cnr.sol
│
├── src/
│   ├── buffer.c
│   ├── buffer.h
│   ├── grafo.c
│   ├── grafo.h
│   ├── main.c
│   ├── pagerank_utils.c
│   ├── pagerank_utils.h
│   ├── xerrori.c
│   └── xerrori.h
│
├── makefile
└── README.md
```

### Main components

#### `main.c`

Contains the main program flow:

1. Parse command-line arguments.
2. Initialize the shared buffer and synchronization primitives.
3. Read the graph dimensions.
4. Start the consumer threads.
5. Produce graph edges from the input file.
6. Wait for the consumer threads.
7. Trim the graph.
8. Run the PageRank computation.
9. Display the results.
10. Release allocated resources.

#### `grafo.c / grafo.h`

Responsible for graph management, including:

* graph initialization;
* edge insertion;
* duplicate detection;
* dead-end detection;
* memory management;
* extraction of the top-`K` PageRank values.

#### `buffer.c / buffer.h`

Implements the producer-consumer mechanism used during graph construction.

The buffer is shared between one producer and multiple consumer threads and is protected using semaphores and a mutex.

#### `pagerank_utils.c / pagerank_utils.h`

Contains the PageRank implementation and the synchronization barrier.

The main PageRank worker functions handle:

* dead-end contributions;
* intermediate vector computation;
* PageRank updates;
* convergence error;
* thread synchronization.

#### `xerrori.c / xerrori.h`

Provides wrappers around system calls and POSIX thread/synchronization functions with error checking.

It also contains command-line argument parsing and the input graph dimension reader.

## Memory Management

The implementation dynamically allocates the main graph structures and the vectors required by the PageRank computation.

After the graph has been constructed, incoming-edge arrays are resized to match their actual number of elements.

At the end of the execution, the program releases:

* graph memory;
* the shared input buffer;
* PageRank vectors;
* the top-`K` result array;
* mutexes;
* semaphores;
* the PageRank barrier.

## Technologies

* **C11**
* **POSIX Threads (`pthread`)**
* **POSIX Semaphores**
* **Condition Variables**
* **Mutexes**
* **GNU Make**
* Dynamic memory management with `malloc`, `realloc`, and `free`

## Purpose

This project was developed as an exercise in implementing a computationally intensive graph algorithm while applying concepts related to **concurrent programming and synchronization**.

The implementation provides practical experience with:

* multithreading;
* shared-memory parallelism;
* producer-consumer architectures;
* synchronization primitives;
* race-condition prevention;
* barriers;
* dynamic data structures;
* iterative numerical algorithms.

## Author

**CorvoScuro27**

GitHub: https://github.com/CorvoScuro27

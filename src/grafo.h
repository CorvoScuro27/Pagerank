#ifndef GRAFO_H
#define GRAFO_H

#include <stdbool.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "xerrori.h"

typedef struct
{
    double value;
    int index;
} Pair;

typedef struct
{
    int i;
    int j;
} edges;

typedef struct
{
    int *nodes;   // array di nodi che puntano a questo nodo
    int elem;     // numero di nodi in questa lista
    int capacity; // capacità dell'array
} inmap;

typedef struct
{
    int N;     // numero dei nodi del grafo
    int *out;  // array con il numero di archi uscenti da ogni nodo
    inmap *in; // array con gli insiemi di archi entranti in ogni nodo
} grafo;

// Funzioni per gestire il grafo
bool duplicate(grafo *g, int i, int j);
void trim_graph(grafo *g);
int add_edge(grafo *g, int i, int j);
void graph_destroy(grafo *g, int n);
grafo inizialize_graph(int n);
void print_graph(grafo *g);
int dead_end_counter(grafo *g);
int compare(const void *a, const void *b);
Pair *get_top_k(double *array, int n, int k);

#endif

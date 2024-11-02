#ifndef PAGERANK_H
#define PAGERANK_H

#include "grafo.h"

typedef struct
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int threads_required;
    int threads_left;
    unsigned int cycle;
} barrier;

typedef struct
{
    grafo *g;
    double *x;
    double *x_next;
    double *y;
    double *err;
    double first_term;
    int start; // Indice di inizio per questo thread
    int end;   // Indice di fine per questo thread
    double d;  // Damping factor
    int taux;
    int maxiter;
    double eps;
    int *iter;
    barrier *barrier;
} thread_data_pr;

double *pagerank(grafo *g, double d, double eps, int maxiter, int taux, int *numiter);
void barrier_init(barrier *barrier, pthread_mutex_t mutex, pthread_cond_t cond, int count);
void barrier_wait(barrier *barrier);
void barrier_destroy(barrier *barrier);
double compute_de(grafo *g, double *x, double d);
void compute_yj(int start, int end, grafo *g, double *y, double *x);
double compute_sum(double *y, grafo *g, int start, int end, int node);
void compute_xj(double *y, double dead_end_factor, int start, int end, double *x_next, double first_term, double d, grafo *g);
double compute_err(double *x, double *x_next, int start, int end);

#endif
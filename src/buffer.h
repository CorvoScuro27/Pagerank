

#ifndef BUFFER_H
#define BUFFER_H

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include "grafo.h"
#include "xerrori.h"

#define BUFFER_SIZE 20

typedef struct
{
    edges *buffer;
    int *consumer_index;
    sem_t *sem_free_slots;
    sem_t *sem_data_items;
    pthread_mutex_t *mutex_buf;
    grafo *g;
    int *edge_num;
} thread_data;

// Funzioni per il produttore e il consumatore
void *consumer(void *d);
void *producer(FILE *f, edges *buffer, sem_t *data_items, sem_t *free_slots, pthread_mutex_t *mu, int T);

#endif 

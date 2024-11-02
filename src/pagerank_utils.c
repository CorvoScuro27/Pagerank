#include "grafo.h"
#include "xerrori.h"
#include "pagerank_utils.h"
#include <math.h>

// funzione per l'inizializzazione della barriera
void barrier_init(barrier *barrier, pthread_mutex_t mutex, pthread_cond_t cond, int count)
{
    barrier->threads_required = count;
    barrier->threads_left = count;
    barrier->cycle = 0;
    barrier->mutex = mutex;
    barrier->cond = cond;
}


// Funzione che mi permette di far attendere i thread finchè non sono arrivati tutti
void barrier_wait(barrier *barrier)
{
    // Il thread entra e prende il mutex
    pthread_mutex_lock(&barrier->mutex);

    // decremento il numero di thread rimanenti

    if (--barrier->threads_left == 0) //se sono l'ultimo thread
    {
        // aumento di 1 cycle
        barrier->cycle++;
        barrier->threads_left = barrier->threads_required; // resetto il contatore de thread rimanenti

        pthread_cond_broadcast(&barrier->cond); // sveglio tutti gli altri thread in attesa
        pthread_mutex_unlock(&barrier->mutex); // rilacio il mutex

        return;
    }
    else
    {
        unsigned int cycle = barrier->cycle; // mi salvo il ciclo corrente

        // ciclo finche il 'ciclo corrente' non cambia (cambia solo quando un ultimo thread lo aggiorna)
        while (cycle == barrier->cycle)
            pthread_cond_wait(&barrier->cond, &barrier->mutex); //utilizzo una condition variable per non fare polling

        // se esco vuol dire che il ciclo è cambiato e che tutti i thread si sono sincronizzati
        pthread_mutex_unlock(&barrier->mutex);
        return;
    }
}

// funzione per distruggere la barriera
void barrier_destroy(barrier *b)
{
    // Distrugge il mutex
    xpthread_mutex_destroy(&b->mutex, QUI);

    // Distrugge la condition variable
    xpthread_cond_destroy(&b->cond, QUI);
}

// funzione che mi permette di calcolare il termine della formula con la sommatoria su gli elementi dell'array x
double compute_de(grafo *g, double *x, double d)
{
    double sumt = 0;
    for (int i = 0; i < g->N; i++)
    {
        if (g->out[i] == 0)
        {
            sumt += x[i];
        }
    }
    double de = (d / g->N) * sumt;
    return de;
}

// funzione che utilizzo per calcolare gli elementi dell'array y
void compute_yj(int start, int end, grafo *g, double *y, double *x)
{
    for (int i = start; i < end; i++)
    {
        if (g->out[i] > 0)
        {
            y[i] = x[i] / g->out[i];
        }
        else
        {
            y[i] = 0; // Gestione di nodi senza archi uscenti
        }
    }
}

// funzione che mi calcola il termine con la sommatoria nella formula del pagerank
double compute_sum(double *y, grafo *g, int start, int end, int node)
{
    double sum = 0.0;

    for (int k = 0; k < g->in[node].elem; k++)
    {
        sum += y[g->in[node].nodes[k]];
    }

    return sum;
}

// funzione per calcolarmi gli elementi di x_next
void compute_xj(double *y, double dead_end_factor, int start, int end, double *x_next, double first_term, double d, grafo *g)
{
    for (int i = start; i < end; i++)
    {
        x_next[i] = first_term + (d * compute_sum(y, g, start, end, i)) + dead_end_factor;
    }
}

// funzione per calcolarmi l'errore
double compute_err(double *x, double *x_next, int start, int end)
{
    double err = 0.0;
    for (int i = start; i < end; i++)
    {
        err += fabs(x_next[i] - x[i]); // Usa fabs per il valore assoluto dei double
    }
    return err;
}

void *worker(void *d)
{
    thread_data_pr *data = (thread_data_pr *)d;

    // Inizializza x  e x_next
    for (int i = data->start; i < data->end; i++)
    {
        data->x[i] = 1.0 / data->g->N;
        data->y[i] = 0.0;
        data->x_next[i] = 0.0;
    }
    int iter = 0;
    while (iter <= data->maxiter)
    {
        iter++;
        // Sincronizzazione
        barrier_wait(data->barrier);

        // Calcolo di dead_end_factor
        double dead_end_factor = compute_de(data->g, data->x, data->d);

        // Sincronizzazione
        barrier_wait(data->barrier);

        // Calcolo di y
        compute_yj(data->start, data->end, data->g, data->y, data->x);

        // Sincronizzazione
        barrier_wait(data->barrier);

        // Calcolo di x_next
        compute_xj(data->y, dead_end_factor, data->start, data->end, data->x_next, data->first_term, data->d, data->g);

        // Sincronizzazione
        barrier_wait(data->barrier);

        // Calcolo errore
        double temp = compute_err(data->x, data->x_next, data->start, data->end);

        xpthread_mutex_lock(&((data->barrier)->mutex), QUI);
        *(data->err) += temp; // Accumula errore
        xpthread_mutex_unlock(&((data->barrier)->mutex), QUI);

        // Sincronizzazione
        barrier_wait(data->barrier);

        // Se l'errore è maggiore della soglia, interrompi l'esecuzione
        if (*(data->err) < data->eps)
        {
            break;
        }

        // Sincronizzazione
        barrier_wait(data->barrier);

        // Resetta l'errore per la prossima iterazione
        xpthread_mutex_lock(&((data->barrier)->mutex), QUI);
        *(data->err) = 0;
        xpthread_mutex_unlock(&((data->barrier)->mutex), QUI);

        // Aggiornamento di x
        for (int i = data->start; i < data->end; i++)
        {
            data->x[i] = data->x_next[i];
        }
    }
    
    *(data->iter) = iter;
    pthread_exit(NULL);
}

double *pagerank(grafo *g, double d, double eps, int maxiter, int taux, int *numiter)
{
    // Dichiaro e inizializzo tutte le risorse che mi servono per i threa
    double *x = malloc(g->N * sizeof(double));
    double *y = malloc(g->N * sizeof(double));
    double *x_next = malloc(g->N * sizeof(double));
    double first_term = (1 - d) / g->N;
    double err = 0.0;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
    barrier barrier;
    barrier_init(&barrier, mutex, cond, taux);

    pthread_t threads[taux];
    thread_data_pr threads_args[taux];

    for (int i = 0; i < taux; i++)
    {
        threads_args[i].g = g;
        threads_args[i].d = d;
        threads_args[i].x = x;
        threads_args[i].y = y;
        threads_args[i].x_next = x_next;
        threads_args[i].first_term = first_term;
        threads_args[i].start = (g->N / taux) * i;
        threads_args[i].end = (i == taux - 1) ? g->N : (g->N / taux) * (i + 1);
        threads_args[i].taux = taux;
        threads_args[i].maxiter = maxiter;
        threads_args[i].barrier = &barrier;
        threads_args[i].err = &err;
        threads_args[i].eps = eps;
        threads_args[i].iter = numiter;

        xpthread_create(&threads[i], NULL, worker, &threads_args[i], QUI);
    }

    for (int i = 0; i < taux; i++)
    {
        pthread_join(threads[i], NULL);
    }

    // libero la memoria, non libero x perchè lo restituisco
    barrier_destroy(&barrier);
    free(y);
    free(x_next);

    return x;
}

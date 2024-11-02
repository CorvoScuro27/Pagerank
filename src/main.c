#include "xerrori.h"
#include "grafo.h"
#include "buffer.h"
#include "pagerank_utils.h"

#define QUI __LINE__, __FILE__

int main(int argc, char *argv[])
{
    int K, M, T;
    double D, E;
    char *infile;

    // Parsing degli argomenti
    parse_arguments(argc, argv, &K, &M, &D, &E, &T, &infile);

    // Gestione del buffer condiviso
    edges *buffer = malloc(BUFFER_SIZE * sizeof(edges));
    int consumer_index = 0;

    // Thread consumatori
    pthread_t t[T];
    thread_data d[T];

    // Semafori
    sem_t sem_free_slots;
    sem_t sem_data_items;

    xsem_init(&sem_free_slots, 0, BUFFER_SIZE, QUI);
    xsem_init(&sem_data_items, 0, 0, QUI);

    // Mutex
    pthread_mutex_t mutex_buf = PTHREAD_MUTEX_INITIALIZER;

    // Inizializzo il grafo
    int n;
    FILE *f = read_graph_dimensions(infile, &n);
    grafo g = inizialize_graph(n);
    int edge_num = 0;

    // Inizializzo i dati dei consumatori
    for (int i = 0; i < T; i++)
    {
        d[i].buffer = buffer;
        d[i].sem_data_items = &sem_data_items;
        d[i].sem_free_slots = &sem_free_slots;
        d[i].mutex_buf = &mutex_buf;
        d[i].consumer_index = &consumer_index;
        d[i].g = &g;
        d[i].edge_num = &edge_num;
    }

    for (int i = 0; i < T; i++)
        xpthread_create(&t[i], NULL, consumer, &d[i], QUI);

    // Leggo le righe del file
    producer(f, buffer, &sem_data_items, &sem_free_slots, &mutex_buf, T);

    // Attendi che i thread consumatori finiscano
    for (int i = 0; i < T; i++)
        pthread_join(t[i], NULL);

    // Fa in modo di sprecare meno spazio
    trim_graph(&g);

    int numiter = 0; // valore di ritorno del numero dell iterazione

    // stampa e calcolo del risultato

    printf("Number of nodes: %d\n", g.N);
    printf("Number of dead-end nodes: %d\n", dead_end_counter(&g));
    printf("Number of valid arcs: %d\n", edge_num);

    double *result = pagerank(&g, D, E, M, T, &numiter);
    double sumt = 0.0;

    for (int i = 0; i < g.N; i++)
    {
        sumt += result[i];
    }
    Pair *top_k = get_top_k(result, g.N, K);

    if (numiter > M)
        printf("Did not converge after %d iterations\n", M);
    else
        printf("Converged after %d iterations\n", numiter);
    printf("Sum of ranks: %.4f (should be 1)\n", sumt);
    printf("Top %d nodes:\n ", K);
    for (int i = 0; i < K - 1; i++)
    {
        printf("%d %f\n ", top_k[i].index, top_k[i].value);
    }
    printf("%d %f", top_k[K - 1].index, top_k[K - 1].value);


    // Libera la memoria allocata
    graph_destroy(&g, n);
    free(buffer);
    free(result);
    free(top_k);

    // Distruggi il mutex
    xpthread_mutex_destroy(&mutex_buf, QUI);
    // Distruggi i semafori
    xsem_destroy(&sem_free_slots, QUI);
    xsem_destroy(&sem_data_items, QUI);

    return 0;
}

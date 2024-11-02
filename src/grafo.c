#include "grafo.h"

// funzione che mi controlla se un arco è duplicato (già presente nel grafo)
bool duplicate(grafo *g, int i, int j)
{
    for (int k = 0; k < g->in[j].elem; k++)
    {
        if (g->in[j].nodes[k] == i)
            return true;
    }
    return false;
}

// Funzione che mi sistema la lunghezza degli array in modo da non sprecare memoria
void trim_graph(grafo *g)
{
    for (int i = 0; i < g->N; i++)
    {
        g->in[i].nodes = realloc(g->in[i].nodes, g->in[i].elem * sizeof(int));
    }
}

// Funzione che mi aggiunge un arco al grafo
int add_edge(grafo *g, int i, int j)
{
    if (!duplicate(g, i, j))
    {
        g->out[i] += 1;
        if (g->in[j].capacity == g->in[j].elem)
        {
            g->in[j].capacity *= 2;
            g->in[j].nodes = realloc(g->in[j].nodes, g->in[j].capacity * sizeof(int));
            if (g->in[j].nodes == NULL)
                termina("Error during realloc");
        }
        g->in[j].nodes[g->in[j].elem] = i;
        g->in[j].elem++;

        return 1;
    }
    else
    {
        return 0;
    }
}

// funzione che mi distrugge il grafo
void graph_destroy(grafo *g, int n)
{
    for (int i = 0; i < n; i++)
    {
        free(g->in[i].nodes);
    }
    free(g->in);
    free(g->out);
}

// Funzione che mi inizializza il grafo
grafo inizialize_graph(int n)
{
    grafo g;
    g.N = n;
    g.out = malloc(n * sizeof(int));
    g.in = malloc(n * sizeof(inmap));

    for (int i = 0; i < n; i++)
    {
        g.in[i].capacity = 10;
        g.in[i].elem = 0;
        g.in[i].nodes = malloc(g.in[i].capacity * sizeof(int));
        if (g.in[i].nodes == NULL)
            termina("Error during malloc");
        g.out[i] = 0; // Inizializza gli archi uscenti
    }

    return g;
}

// funzione che mi stampa il grafo utile per il debugging
void print_graph(grafo *g)
{
    for (int i = 0; i < g->N; i++)
    {
        printf("Node %d:", i);
        for (int j = 0; j < g->in[i].elem; j++)
        {
            printf(" <- %d", g->in[i].nodes[j]);
        }
        printf("\n");
    }
}

// Funzione che conta quanti nodi dead-end ci sono nel grafo
int dead_end_counter(grafo *g)
{
    int sum = 0;
    for (int i = 0; i < g->N; i++)
        if (g->out[i] == 0)
            sum += 1;
    return sum;
}

// Funzione utilizzata per il qsort
int compare(const void *a, const void *b)
{
    double diff = ((Pair *)b)->value - ((Pair *)a)->value;
    if (diff > 0)
        return 1;
    if (diff < 0)
        return -1;
    return 0;
}

// Funzione che mi restituisce i K nodi con valore più alto di pagerank
Pair *get_top_k(double *array, int n, int k)
{
    // Alloco un array di coppie (valore, indice)
    Pair *pairs = (Pair *)malloc(n * sizeof(Pair));

    // riempio l'array do coppie
    for (int i = 0; i < n; i++)
    {
        pairs[i].value = array[i];
        pairs[i].index = i;
    }

    // Ordino l'array di coppie in ordine decrescente
    qsort(pairs, n, sizeof(Pair), compare);

    // Creo un nuovo array per i primi k elementi
    Pair *top_k = (Pair *)malloc(k * sizeof(Pair));

    // Copio i primi k elementi
    for (int i = 0; i < k; i++)
    {
        top_k[i].index = pairs[i].index;
        top_k[i].value = pairs[i].value;
    }

    // Libero la memoria temporanea
    free(pairs);

    return top_k;
}
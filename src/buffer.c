
#include "buffer.h"

// funzione che corrisponde al thread consumatore
void *consumer(void *d)
{
    thread_data *data = (thread_data *)d;
    edges edge;
    while (true)
    {
        // utilizzo un semaforo per capire se ci sono dati nel buffer
        xsem_wait(data->sem_data_items, QUI);

        // prima di entrare nella sezione critica utilizzo un mutex, cosi da evitare race conditions
        xpthread_mutex_lock(data->mutex_buf, QUI);

        // leggo l'arco dal buffer
        edge = data->buffer[*(data->consumer_index)];
        *(data->consumer_index) = (*(data->consumer_index) + 1) % BUFFER_SIZE;
        if (edge.i == -1 && edge.j == -1)
        {
            // qui il produttore ha finito di "produrre" dati e io ho finito di leggerli
            xpthread_mutex_unlock(data->mutex_buf, QUI);
            xsem_post(data->sem_free_slots, QUI);
            break;
        }
        if (edge.i != edge.j)
        {
            // aggiungo l'arco al grafo se non è un loop
            *(data->edge_num) += add_edge(data->g, edge.i, edge.j);
        }
        // rilascio il mutex
        xpthread_mutex_unlock(data->mutex_buf, QUI);
        // comunico con il semaforo che si è "liberata una posizione"
        xsem_post(data->sem_free_slots, QUI);
    }
    // esco, ho finito
    pthread_exit(NULL);
}

void *producer(FILE *f, edges *buffer, sem_t *data_items, sem_t *free_slots, pthread_mutex_t *mu, int T)
{
    // indice del consumatore, utilizzo una variabile locale tanto il producer è solamente uno
    int producer_index = 0;
    while (true)
    {
        // leggo i due interi (arco) dal file che ho già aperto
        int i, j;
        int e = fscanf(f, "%d %d", &i, &j);
        if (e == EOF)
            break;
        if (e != 2)
            termina("Error during reading the edges.");

        // utilizzo un semaforo per capire se ci sono "posti liberi" nel buffer
        xsem_wait(free_slots, QUI);
        // uso un mutex per entrare nella sezione critica
        xpthread_mutex_lock(mu, QUI);

        // posiziono gli interi e aggiorno la posizione dell'indice
        buffer[producer_index].i = i - 1;
        buffer[producer_index].j = j - 1;
        producer_index = (producer_index + 1) % BUFFER_SIZE;

        // esco dalla sezione critica rilasciando il mutex
        xpthread_mutex_unlock(mu, QUI);
        // comunico tramite il semaforo che ho posizionato un nuovo elemento nel buffer
        xsem_post(data_items, QUI);
    }

    // ho finito di produrre dati:
    // metto -1 nel buffer per far capire hai consumatori che ho finito di leggere i dati
    for (int k = 0; k < T; k++)
    {
        xsem_wait(free_slots, QUI);
        xpthread_mutex_lock(mu, QUI);

        buffer[producer_index].i = -1;
        buffer[producer_index].j = -1;
        producer_index = (producer_index + 1) % BUFFER_SIZE;

        xpthread_mutex_unlock(mu, QUI);
        xsem_post(data_items, QUI);
    }
    // chiudo il file
    fclose(f);
    return NULL;
}

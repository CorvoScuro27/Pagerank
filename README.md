# Implementazione dell'Algoritmo PageRank in C

## Indice

1. [Introduzione](#introduzione)
2. [Cosa è il PageRank?](#cosa-e-il-pagerank)
3. [Struttura del Codice](#struttura-del-codice)
   - [File di intestazione e librerie necessarie](#file-di-intestazione-e-librerie-necessarie)
   - [Strutture Dati](#strutture-dati)
4. [Dettagli delle Funzioni](#dettagli-delle-funzioni)
   - [Funzioni di Inizializzazione della Barriera](#funzioni-di-inizializzazione-della-barriera)
   - [Calcolo del PageRank](#calcolo-del-pagerank)
   - [Sincronizzazione e Threading](#sincronizzazione-e-threading)
5. [Esecuzione e Compilazione](#esecuzione-e-compilazione)
7. [Considerazioni Finali](#considerazioni-finali)

## Introduzione

L'algoritmo PageRank è stato sviluppato da Larry Page e Sergey Brin durante il loro dottorato all'Università di Stanford nel 1996. Questo algoritmo ha rivoluzionato il modo in cui i motori di ricerca classificano le pagine web, basandosi non solo sul contenuto di una pagina ma anche sulla struttura dei link tra le pagine stesse. Questa implementazione utilizza il linguaggio C e threading per ottimizzare le performance nel calcolo del PageRank su grafi di grandi dimensioni.

## Cosa è il PageRank?

### 1. Definizione

Il PageRank è un algoritmo che assegna un punteggio a ciascun nodo in un grafo, basato sulla qualità e quantità dei link che puntano a quel nodo. È stato progettato per determinare l'importanza relativa delle pagine web, e il suo principio di base è che le pagine più importanti sono quelle che sono linkate da molte altre pagine importanti.

### 2. Formula del PageRank

La formula del PageRank può essere espressa come segue:

\[ PR(A) = (1 - d) + d \sum_{i=1}^{k} \frac{PR(B_i)}{C(B_i)} \]

Dove:
- \( PR(A) \) è il PageRank della pagina A.
- \( d \) è il fattore di damping (solitamente impostato a 0.85).
- \( B_i \) sono le pagine che linkano alla pagina A.
- \( C(B_i) \) è il numero totale di link in uscita dalla pagina \( B_i \).

### 3. Intuizione

L'idea è che se una pagina A è linkata da molte altre pagine (soprattutto quelle con un alto PageRank), allora A è considerata più importante. Questo crea un sistema di valutazione che riflette l'importanza delle informazioni presenti nel web.

## Struttura del Codice

### File di intestazione e librerie necessarie

Il codice richiede alcune librerie e file di intestazione per funzionare correttamente. Queste includono:

- **grafo.h**: definisce la struttura del grafo e le relative funzioni.
- **xerrori.h**: gestisce gli errori e le eccezioni.
- **pagerank_utils.h**: contiene funzioni ausiliarie utilizzate nell'algoritmo PageRank.
- Librerie standard come `<math.h>` e `<pthread.h>` per il calcolo matematico e la gestione dei thread.

### Strutture Dati

Le strutture dati sono fondamentali per gestire le informazioni nel grafo e per implementare l'algoritmo PageRank. La seguente struttura `grafo` è un esempio:

```c
typedef struct {
    int N; // Numero di nodi
    int *out; // Array per i link uscenti
    InLinks *in; // Struttura per i link entranti
} grafo;

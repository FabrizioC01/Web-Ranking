# Web Ranking
Il progetto è suddiviso su due cartelle: una cartella `error/` che contiene tutti i file riguardanti la gestione degli errori, e una cartella `core/` contenente tutto ciò che riguarda l'algoritmo, quindi la creazione del grafo che avviene nel file `graph.c`, e il calcolo del pagerank che avviene nel file `calc.c`. In questi file ci sono le funzioni che sucessivamente saranno chiamati nel `main.c` per il la creazione, il calcolo e in fine la liberazione della memoria.
## Creazione grafo - *graph.c*
La funzione che viene chiamata nel main per la creazione del grafo è
```C
graph *graph_init(const int threads, FILE *infile)
```
che restituisce il grafo popolato con i valori letti da `infile`.  

Per la **popolazione** del grafo vengono creati dei threads il cui numero è ricevuto come parametro della funzione, il passaggio dei dati avviene attraverso un buffer sfruttando quindi lo schema produttore/consumatore.  
Il **main** thread ricopre il ruolo di  **produttore** leggendo i valori dal file `infile` e aggiungendoli a una `struct pair` contenente le coppie ordinate che verranno aggiunte al buffer.  
La concorrenza è gestita attraverso due semafori "legati" a una mutex
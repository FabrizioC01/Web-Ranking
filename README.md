# Web Ranking
Il progetto è suddiviso su due cartelle: una cartella `error/` che contiene tutti i file riguardanti la gestione degli errori, e una cartella `core/` contenente tutto ciò che riguarda l'algoritmo, quindi la creazione del grafo che avviene nel file `graph.c`, e il calcolo del pagerank che avviene nel file `calc.c`. In questi file ci sono le funzioni che sucessivamente saranno chiamati nel `main.c` per il la creazione, il calcolo e in fine la liberazione della memoria.
## Creazione grafo - *graph.c*
La funzione che viene chiamata nel main per la creazione del grafo è
```C
graph *graph_init(const int threads, FILE *infile)
```
che restituisce il grafo popolato con i valori letti da `infile`.
### Produttore
Per la **popolazione** del grafo vengono creati dei threads il cui numero è ricevuto come parametro della funzione, il passaggio dei dati avviene attraverso un buffer sfruttando quindi lo schema produttore/consumatore.  
Il **main** thread ricopre il ruolo di  **produttore** leggendo i valori dal file `infile` e aggiungendoli a una `struct pair` contenente le coppie ordinate che verranno aggiunte al buffer.  
La concorrenza è gestita attraverso due semafori "legati" a una mutex
```C
sem_t free_slots;
sem_t busy_slots;
pthread_mutex_t mutex;
```
utili a tenere traccia degli slot liberi e occupati del buffer. Una volta terminata la lettura dei valori dal file, il produttore (*main thread*) manda tanti valori di terminazione (-1), tanti quanti sono i thread che sono stati creati.
### Consumatori
I **Consumatori** stanno in wait su `busy_slots` in attesa che siano dei valori nel buffer, se ci sono acquisiscono la mutex, salvano la coppia, rilasciano la mutex e fanno la post su `free_slots`. Fatto ciò si occupano adesso dell aggiunta al grafo, e per evitare race condition con gli altri thread, per la modifica dei valori del grafo
viene utilizzata una ulteriore mutex chiamata `g_mutex`.
## Calcolo del pagerank - *calc.c*
Il calcolo del pagerank sfrutta lo stesso numero di thread utilizzati per la creazione del grafo, all'inizio della funzione viene creato un thread apposito per la gestione del segnale `SIGUSR1` e per terminare il thread handler ho fatto in modo che venga catturato anche `SIGUSR2` .  
In seguito vengono create le strutture dati (array, indici, contatori etc...) e i thread che dovranno svolgere l'effettivo calcolo.  
La gestione della concorrenza è stata fatta attraverso **condition variables**, visto che i valori da passare ai thread ausiliari ("consumatori") erano interi da 0 a N-1.
```C
pthread_mutex_t mutex;
pthread_cond_t can_update;
pthread_cond_t lib;
```
### Produttore
Il thread **produttore**(*main*) si mette in attesa sulla condition variable `can_update` e testa la variabile `endedt`.  
La variabile `endedt` conta i thread che hanno effettivamente terminato la computazione dell' i-esima componente del vettore, mentre un altra variabile(`index`) sarà effettivamente consumata dai thread e subito rilasciata.  
Una volta uscito dalla condizione dalla condizione il produttore:
<ol>
<li>Azzera la variabile index</li>
<li>Azzera la variabile endedt</li>
<li>Scambia il vettore X con NEXT</li>
<li>Imposta la variabile nella quale è memorizzato il contributo dei nodi Dead-end a -1</li>
</ol>  

Tutto ciò avviene in un ciclo while controllando il numero di iterazioni passato come argomento nella chiamata e a ogni iterazione verifico che l'errore calcolato dai thread ausiliari diventi minore di quello passato negli argomenti, in caso tal caso esco dal ciclo.
Terminate le iterazioni passo i valori di terminazione ai thread ausiliari e termino il programma.  
### Consumatori


![](https://img.shields.io/badge/C%20Language-grey?style=for-the-badge&logo=C)
![](https://img.shields.io/badge/Python-yellow?style=for-the-badge&logo=Python)
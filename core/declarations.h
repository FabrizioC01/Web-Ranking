#define _GNU_SOURCE 
#include <ctype.h>  
#include <stdio.h>    
#include <stdlib.h>   
#include <stdbool.h>  
#include <assert.h>   
#include <string.h>   
#include <errno.h>    
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>        
#include <fcntl.h>           
#include <pthread.h>

// Costanti iniziali che sostutuiscono i valori non inseriti
#define K_NODES 3
#define MAX_ITER 100
#define DMP_FACT 0.9
#define MAX_ERR 1.0e7
#define N_THREADS 3

#define INIT_INPUT_N 20

//Costante dimensione buffer prod/cons
#define BUFF_SIZE 30

#define USAGE_MSG "\npagerank [-k K] [-m M] [-d D] [-e E] infile \n\npositional arguments:\n   infile   input file\n\noptions:\n -k K   show top K nodes (default 3)\n -m M   maximum number of iterations (default 100)\n -d D   damping factor (default 0.9)\n -e E   max error (default 1.0e7)\n -t T   number of threads (default 3)\n"

//Lista di nodi
typedef struct nodi{
    int value;
    struct nodi *next;
}inmap;

//Struttura dati grafo
typedef struct grafo{
    int nodi; //totale dei nodi
    int *out; //array contenente il numero di archi uscenti dal nodo i (dimensione array=nodi)
    inmap **in; //array di liste con valori dei nodi degli archi entranti al nodo i (dimensione array=nodi) 
} graph;


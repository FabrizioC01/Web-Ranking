#include "graph.h"

#define pos __LINE__,__FILE__

typedef struct{
    int l;
    int r;
} pair;

typedef struct dati{
    pair *buffer;
    int *pointer;
    pthread_mutex_t *mu;
    sem_t *f_slots;
    sem_t *b_slots;
} dati;

void consumer_routine(void *data){
    dati d = *(dati *)data;
    
}

void start_graph(const int nodes,const int threads,const int iter,const int damp,const int m_err, FILE *infile){
    pair buffer[BUFF_SIZE];
    ssize_t e;
    int index=0,row=0,col=0,edges=0;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    sem_t free_slots;
    sem_t busy_slots;

    

    char *line;
    ssize_t s;
    //Lettura della prima linea
    while(true){
        e = getline(&line,&s,infile);
        if(e==EOF) break;
        if(line[0]!='%'){
            sscanf(line,"%d %d %d",&row,&col,&edges);
            break;
        }
    } 
    if(row!=col) raise_error("Errore il file non contiene una matrice di adiacenza",pos);

    dati d[N_THREADS];
    pthread_t t[N_THREADS];
    for(int i=0;i<N_THREADS;i++){
        d[i].buffer = &buffer;
        d[i].mu= &mutex;
        d[i].pointer=&index;
        d[i].b_slots=&busy_slots;
        d[i].f_slots=&free_slots;
        pthread_create(&t[i],NULL,&consumer_routine,&d[i]);
    }

    int from,to;

    while(true){
        e = getline(&line,&s,infile);
        if(e==EOF){
            for(int i=0;i<N_THREADS;i++){
                sem_wait(&free_slots);
                pthread_mutex_lock(&mutex);
                buffer[index].l=-1;
                buffer[index].r=-1;
                index++;
                pthread_mutex_unlock(&mutex);
                sem_post(&busy_slots);
            } 
            break;
        }
        if(line[0]!='%'){
            sscanf(line,"%d %d",&from,&to);
            sem_wait(&free_slots);
            pthread_mutex_lock(&mutex);
            buffer[index].l=from;
            buffer[index].r=to;
            index++;
            pthread_mutex_unlock(&mutex);
            sem_post(&busy_slots);
        }
    }
}
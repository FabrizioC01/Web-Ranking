#include "graph.h"

#define pos __LINE__,__FILE__

//Strutture interne per il passaggio dei dati ai threads
typedef struct coppia{
    int l;
    int r;
} pair;

//Strutture interne per il passaggio dei dati ai threads
typedef struct dati{
    pair *buffer;
    int *pointer;
    pthread_mutex_t *mu;
    sem_t *f_slots;
    sem_t *b_slots;
    graph *graph;
    pthread_mutex_t *g_mutex;
} dati;


//Funzione eseguita dai thread consumatori
void *consumer_routine(void *data){
    dati *d = (dati *)data;
    int pointer=3;
    pair coppia; //per salvare i valori temp
    do{
        xsem_wait(d->b_slots,pos);
        xpthread_mutex_lock(d->mu,pos);
        coppia.l=(d->buffer[*d->pointer-1]).l;
        coppia.r=(d->buffer[*d->pointer-1]).r;
        pointer=*d->pointer-1;
        (*d->pointer)-=1;
        xpthread_mutex_unlock(d->mu,pos);
        xsem_post(d->f_slots,pos);

        if(coppia.l!=-1 && coppia.r!=-1 && coppia.l!=coppia.r){
            //aggiungo al grafo
            coppia.l--;
            coppia.r--;

            xpthread_mutex_lock(d->g_mutex,pos);
            
            (d->graph)->nodi+=1;
            (d->graph)->out[coppia.l]+=1;
            inmap *list = (d->graph)->in[coppia.r];
            inmap *n_elem = malloc(sizeof(inmap));
            n_elem->value=coppia.r;
            n_elem->next=list;
            list=n_elem;
            xpthread_mutex_unlock(d->g_mutex,pos);
            coppia.l++;
            coppia.r++;
        }
        
    }while(coppia.l!=-1 && coppia.r!=-1);

    
    pthread_exit(NULL);
}

void start_graph(const int nodes,const int threads,const int iter,const int damp,const int m_err, FILE *infile){

    pair buffer[BUFF_SIZE];
    ssize_t e=0;
    int index=0,row=0,col=0,edges=0;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t graph_mutex = PTHREAD_MUTEX_INITIALIZER;
    sem_t free_slots;
    sem_t busy_slots;

    xsem_init(&free_slots,0,BUFF_SIZE,pos);
    xsem_init(&busy_slots,0,0,pos);

    graph g;

    char *line=NULL;
    ssize_t s=0;

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

    //Inizializzazione grafo vuoto
    g.nodi=0;
    g.out= malloc(sizeof(int)*row);
    g.in= malloc(sizeof(inmap)*row);

    for(int i=0;i<row;i++){
        g.out[i]=0;
        g.in[i]=NULL;
    }

    dati d[N_THREADS];
    pthread_t t[N_THREADS];
    for(int i=0;i<N_THREADS;i++){
        d[i].buffer = buffer;
        d[i].mu= &mutex;
        d[i].pointer=&index;
        d[i].b_slots=&busy_slots;
        d[i].f_slots=&free_slots;
        d[i].graph= &g;
        d[i].g_mutex= &graph_mutex;
        xpthread_create(&t[i],NULL,consumer_routine,&d[i],pos);
    }

    int from,to;

    //Produttore che passa i dati e i valori di terminazione
    while(true){
        e = getline(&line,&s,infile);
        if(e==EOF){
            for(int i=0;i<N_THREADS;i++){
                xsem_wait(&free_slots,pos);
                xpthread_mutex_lock(&mutex,pos);
                buffer[index].l=-1;
                buffer[index].r=-1;
                index++;
                xpthread_mutex_unlock(&mutex,pos);
                xsem_post(&busy_slots,pos);
            } 
            break; 
        }
        if(line[0]!='%'){
            sscanf(line,"%d %d",&from,&to);
            xsem_wait(&free_slots,pos);
            xpthread_mutex_lock(&mutex,pos);
            buffer[index].l=from;
            buffer[index].r=to;
            index++;
            xpthread_mutex_unlock(&mutex,pos);
            xsem_post(&busy_slots,pos);
        }
    }

    for(int i=0;i<N_THREADS;i++){
        xpthread_join(t[i],NULL,pos);
    }
    xsem_destroy(&free_slots,pos);
    xsem_destroy(&busy_slots,pos);
    xpthread_mutex_destroy(&mutex,pos);
    xpthread_mutex_destroy(&graph_mutex,pos);
}
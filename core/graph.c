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
    int *edges;
    int *d_end;
} dati;

//Aggiunge in testa se non esiste già
void push_inmap(inmap **input,int val,int *ed,int *out_count){
    inmap *head =*input;
    while(head!=NULL){
        if(head->value==val) return;
        head=head->next;
    }
    inmap *n_elem = (inmap *)malloc(sizeof(inmap));
    *ed+=1;
    out_count[val]+=1;
    n_elem->value=val;
    n_elem->next=*input;
    *input=n_elem;
}

//Funzione eseguita dai thread consumatori
void *consumer_routine(void *data){
    dati *d = (dati *)data;
    pair coppia; //per salvare i valori temp
    do{

        xsem_wait(d->b_slots,pos);
        xpthread_mutex_lock(d->mu,pos);
        coppia.l=(d->buffer[*d->pointer % BUFF_SIZE]).l;
        coppia.r=(d->buffer[*d->pointer % BUFF_SIZE]).r;
        (*d->pointer)+=1;
        xpthread_mutex_unlock(d->mu,pos);
        xsem_post(d->f_slots,pos);

        if(coppia.l==-1 && coppia.r==-1) break;

        if(coppia.l>d->graph->nodi || coppia.r>d->graph->nodi) raise_error("\nErrore trovato arco non valido...",pos);

        if(coppia.l!=coppia.r ){
            //aggiungo al grafo
            xpthread_mutex_lock(d->g_mutex,pos);
            if(d->graph->out[coppia.l-1]==0) (*d->d_end)--;
            push_inmap(&(d->graph)->in[(coppia.r)-1],coppia.l-1,d->edges,d->graph->out);
            xpthread_mutex_unlock(d->g_mutex,pos);
        }

        
    }while(true);

    pthread_exit(NULL);
}

graph *graph_init(const int threads, FILE *infile){

    pair buffer[BUFF_SIZE];
    ssize_t e=0;
    int cindex=0,pindex=0,row=0,col=0,edges=0;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t graph_mutex = PTHREAD_MUTEX_INITIALIZER;
    sem_t free_slots;
    sem_t busy_slots;
    int valid_edges=0,d_end;

    xsem_init(&free_slots,0,BUFF_SIZE,pos);
    xsem_init(&busy_slots,0,0,pos);

    graph *g = malloc(sizeof(graph));

    char *line=NULL;
    size_t s=0;

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
    d_end=row;
    g->nodi=row;
    g->out= malloc(sizeof(int)*row);
    g->in= malloc(sizeof(inmap*)*row);

    for(int i=0;i<row;i++){
        g->out[i]=0;
        g->in[i]=NULL;
    }

    dati d[threads];
    pthread_t t[threads];
    for(int i=0;i<threads;i++){
        d[i].d_end=&d_end;
        d[i].edges=&valid_edges;
        d[i].buffer = buffer;
        d[i].mu= &mutex;
        d[i].pointer=&cindex;
        d[i].b_slots=&busy_slots;
        d[i].f_slots=&free_slots;
        d[i].graph= g;
        d[i].g_mutex= &graph_mutex;
        xpthread_create(&t[i],NULL,consumer_routine,&d[i],pos);
    }

    int from,to;

    //Produttore che passa i dati e i valori di terminazione
    while(true){
        e = getline(&line,&s,infile);
        if(e==EOF){
            for(int i=0;i<threads;i++){
                xsem_wait(&free_slots,pos);
                xpthread_mutex_lock(&mutex,pos);
                buffer[pindex % BUFF_SIZE].l=-1;
                buffer[pindex % BUFF_SIZE].r=-1;
                pindex++;
                xpthread_mutex_unlock(&mutex,pos);
                xsem_post(&busy_slots,pos);
            } 
            break; 
        }
        if(line[0]!='%'){
            sscanf(line,"%d %d",&from,&to);
            xsem_wait(&free_slots,pos);
            xpthread_mutex_lock(&mutex,pos);
            if(from<0 || to<0) raise_error("\nErrore nel file sono stati letti archi non validi...",pos);
            buffer[pindex % BUFF_SIZE].l=from;
            buffer[pindex % BUFF_SIZE].r=to;
            pindex++;
            xpthread_mutex_unlock(&mutex,pos);
            xsem_post(&busy_slots,pos);
        }
    }


    for(int i=0;i<threads;i++){
        xpthread_join(t[i],NULL,pos);
    }
    
    fprintf(stdout,"Number of nodes: %d",g->nodi);
    fprintf(stdout,"\nNumber of dead-end nodes: %d",d_end);
    fprintf(stdout,"\nNumber of valid arcs: %d",valid_edges);

    free(line);
    xsem_destroy(&free_slots,pos);
    xsem_destroy(&busy_slots,pos);
    xpthread_mutex_destroy(&mutex,pos);
    xpthread_mutex_destroy(&graph_mutex,pos);
    return g;
}


void graph_delete(graph **g){
    graph *gr = *g;
    free(gr->out);
    for(int i=0;i<gr->nodi;i++){
        inmap *hd=gr->in[i];
        while(hd!=NULL){
            gr->in[i]=gr->in[i]->next;
            free(hd);
            hd=gr->in[i];
        }
    }
    free(gr->in);
    free(gr);
}
#include "graph.h"

#define pos __LINE__,__FILE__

typedef struct sign{
    double *array;
    int *iter;
    int s;
    pthread_mutex_t *data_mutex;
}signal_data;

typedef struct calc{
    graph *g;
    double damp;
    double *dead_end;
    double *error;
    int *ended;
    //vettori da usare anch'essi in mutua esclusione
    double *X;
    double *Y;
    double *NEXT;
    pthread_cond_t *can_update;
    //utili a capire che indice sta calcolando
    int *index;
    pthread_mutex_t *index_mutex;
    pthread_cond_t *free;
} dati;

void vect_Y(graph *g,double *Y, double *X,double *de){
    for(int i=0;i<g->nodi;i++){
        if(g->out[i]>0){
            Y[i]=X[i]/g->out[i];
        }else{
            *de+=X[i];
        }
    }
}

void *sig_handler(void *val){
    signal_data *data = (signal_data *)val;
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask,SIGUSR1);
    sigaddset(&mask,SIGUSR2);
    int sig;
    while (true)
    {
        int e = sigwait(&mask,&sig);
        if(e!=0) perror("Error sigwait");
        if(sig==SIGUSR1){
            char BUFF[50];
            write(STDERR_FILENO,"\n== SIGUSR1 recived ==",23);
            xpthread_mutex_lock(data->data_mutex,pos);
            ssize_t s1=sprintf(BUFF,"\nIterazione: %d",*data->iter);
            write(STDERR_FILENO,BUFF,s1);
            double max=DBL_MIN;
            int tmp;
            for(int i=0;i<data->s;i++){
                if(data->array[i]>max){ max=data->array[i]; tmp=i;}
            }
            xpthread_mutex_unlock(data->data_mutex,pos);
            ssize_t s2=sprintf(BUFF,"\nBest value: [%d] %f\n",tmp,max);
            write(STDERR_FILENO,BUFF,s2);
        }
        if(sig==SIGUSR2){
            break;
        }
    }
    return NULL;
}

void *thread_job(void *data){
    dati *d= (dati *)data;
    int ind;
    while (true){
        xpthread_mutex_lock(d->index_mutex,pos);
        while(*d->index==d->g->nodi){ pthread_cond_wait(d->free,d->index_mutex);}
        if(*d->index==EOF){
            xpthread_mutex_unlock(d->index_mutex,pos);
            break;    
        }

        ind = *d->index;
        (*d->index)+=1;

        if(*d->dead_end == -1.0){
            *d->error=0.0;
            *d->dead_end=0.0;
            vect_Y(d->g,d->Y,d->X,d->dead_end);
            *d->dead_end*=((d->damp))/d->g->nodi;
        }

        xpthread_mutex_unlock(d->index_mutex,pos);

        d->NEXT[ind]=0.0;
        inmap *hd = d->g->in[ind];
        while(hd!=NULL){
            d->NEXT[ind]=d->NEXT[ind]+d->Y[hd->value];
            hd=hd->next;
        }

        d->NEXT[ind]=d->NEXT[ind]*d->damp;
        d->NEXT[ind]+=(*d->dead_end);
        d->NEXT[ind]+=(1-d->damp)/d->g->nodi;
        

        xpthread_mutex_lock(d->index_mutex,pos);
        *d->error+=fabs(d->NEXT[ind]-d->X[ind]);
        (*d->ended)+=1;
        pthread_cond_signal(d->can_update);
        xpthread_mutex_unlock(d->index_mutex,pos);
    }
    pthread_exit(NULL);
}

double *pagerank(graph *g, double d, double eps, int maxiter, int taux, int *numiter){
    fprintf(stderr,"\nAdesso puoi mandare SIGUSR1...\n");

    pthread_t signal_handler;
    sigset_t mask,oldMask;
    sigemptyset(&mask);
    sigaddset(&mask,SIGUSR1);
    sigaddset(&mask,SIGUSR2);
    pthread_sigmask(SIG_BLOCK,&mask,&oldMask);

    int n_nodi = g->nodi;
    double dead_end=-1.0;
    double error= 0.0;
    int index=n_nodi;
    double damp= d;



    pthread_t thread[taux];

    pthread_mutex_t mutex= PTHREAD_MUTEX_INITIALIZER;

    pthread_cond_t can_update = PTHREAD_COND_INITIALIZER;
    pthread_cond_t lib= PTHREAD_COND_INITIALIZER;

    int endedt=n_nodi;


    double *x= malloc(sizeof(double)*n_nodi);
    double *y= malloc(sizeof(double)*n_nodi);
    double *next= malloc(sizeof(double)*n_nodi);

    //carico il vettore iniziale
    for(int j=0;j<n_nodi;j++){
        x[j]=(double)1/n_nodi;
        next[j]=0.0;
    }
    
    dati *data = (dati *)malloc(sizeof(dati)*taux);
    signal_data s;
    s.array=next;
    s.s=n_nodi;
    s.data_mutex=&mutex;
    int iterazioni=0;
    s.iter=&iterazioni;
    xpthread_create(&signal_handler,NULL,sig_handler,&s,pos);

    for(int i=0;i<taux;i++){
        data[i].error=&error;
        data[i].ended=&endedt;
        data[i].can_update= &can_update;
        data[i].damp=d;
        data[i].g= g;
        data[i].dead_end=&dead_end;
        data[i].free=&lib;
        data[i].index= &index;
        data[i].NEXT= next;
        data[i].index_mutex=&mutex;
        data[i].X = x;
        data[i].Y = y;
        xpthread_create(&thread[i],NULL,thread_job,&data[i],pos);
    }



    while(iterazioni<maxiter){//produce indici di X[i] su cui devono lavorare i thread
        xpthread_mutex_lock(&mutex,pos);
        while(endedt!=n_nodi){ pthread_cond_wait(&can_update,&mutex);}
        if(error<eps && iterazioni!=0){
            pthread_mutex_unlock(&mutex);
            fprintf(stdout,"\nConverged after %d iterations",iterazioni);
            break;
        }
        endedt=0;
        index=0;
        dead_end=-1.0;
        for(int i=0;i<n_nodi && iterazioni!=0;i++){ x[i]=next[i]; }
        iterazioni++;
        pthread_cond_broadcast(&lib);
        xpthread_mutex_unlock(&mutex,pos);
    } 
    if(!(iterazioni<maxiter)){
        fprintf(stdout,"\nDid not converge after %d iterations",iterazioni);
    }
    for(int i=0;i<taux;i++){
        xpthread_mutex_lock(&mutex,pos);
        while(endedt!=n_nodi) pthread_cond_wait(&can_update,&mutex);
        index=EOF; //term value
        pthread_cond_signal(&lib);
        xpthread_mutex_unlock(&mutex,pos);
    }

    *numiter=iterazioni;
    pthread_kill(signal_handler,SIGUSR2);
    pthread_sigmask(SIG_SETMASK,&oldMask,NULL);

    xpthread_join(signal_handler,NULL,pos);
    for(int j=0;j<taux;j++){
        xpthread_join(thread[j],NULL,pos);
    }
    double sum=0.0;

    for(int j=0;j<n_nodi;j++){
        sum+=next[j];
    }

    fprintf(stdout,"\nSum of ranks: %.4f   (should be 1)",sum);
    free(data);
    free(x);
    free(y);
    return next;
}
#include "graph.h"

#define pos __LINE__,__FILE__


typedef struct calc{
    graph *g;
    double damp;
    double teleporting;
    double *dead_end;
    double *error;
    int *ended;
    //vettori da usare anch'essi in mutua esclusione
    double *X;
    double *Y;
    double *NEXT;
    pthread_cond_t *can_update;
    pthread_mutex_t *calc_mutex;
    //utili a capire che indice sta calcolando
    int *index;
    pthread_mutex_t *index_mutex;
    pthread_cond_t *free;
} dati;

void vect_Y(graph *g,double *Y, double *X){
    fprintf(stderr,"\nModifica array Y:");
    for(int i=0;i<g->nodi;i++){
        if(g->out[i]>0){
            fprintf(stderr,"\n{%d} out %d NEW Y %.6f",i,g->out[i],X[i]/g->out[i]);
            Y[i]=X[i]/g->out[i];
        }else{
            Y[i]=0.0;
        }
    }
}

void *thread_job(void *data){
    dati *d= (dati *)data;
    int ind;
    while (true){
        xpthread_mutex_lock(d->index_mutex,pos);
        while(*d->index==d->g->nodi || *d->index==d->g->nodi){ pthread_cond_wait(d->free,d->index_mutex);}
        if(*d->index==EOF){
            xpthread_mutex_unlock(d->index_mutex,pos);
            break;    
        }
        ind = *d->index;
        (*d->index)+=1;

        if(*d->dead_end == -1.0){
            vect_Y(d->g,d->Y,d->X);
            fprintf(stderr,"\nArray Y");
            for(int i=0;i<d->g->nodi;i++){ fprintf(stderr,"\n(%d)-> %f",i,d->Y[i]);}
            *d->dead_end=0.0;
            for(int i=0;i<d->g->nodi;i++){
                if(d->g->out[i]==0) *d->dead_end+=d->X[i];
            }
            *d->dead_end*=((d->damp))/d->g->nodi;
        }
        xpthread_mutex_unlock(d->index_mutex,pos);

        d->NEXT[ind]=0.0;
        inmap *hd = d->g->in[ind];
        fprintf(stderr,"\nSum for %d",ind);
        while(hd!=NULL){
            fprintf(stderr,"Value of Y[%d] %f | ",hd->value,d->Y[hd->value]);
            d->NEXT[ind]=d->NEXT[ind]+d->Y[hd->value];
            hd=hd->next;
        }
        d->NEXT[ind]=d->NEXT[ind]*0.9;
        
        fprintf(stderr,"\nSum :[%d] %.9f",ind,d->NEXT[ind]);

        d->NEXT[ind]+=(*d->dead_end);
        d->NEXT[ind]+=(1-d->damp)/d->g->nodi;

        xpthread_mutex_lock(d->index_mutex,pos);
        

        fprintf(stderr,"\nWake up from sleep with value of : %d | damp: %f | dead_end: %f | telep: %f",ind,d->damp,*d->dead_end,(1-d->damp)/d->g->nodi);
        (*d->ended)+=1;
        pthread_cond_signal(d->can_update);
        xpthread_mutex_unlock(d->index_mutex,pos);
    }
    fprintf(stderr,"\nTermino...");
    pthread_exit(NULL);
}

double *pagerank(graph *g, double d, double eps, int maxiter, int taux, int *numiter){
    fprintf(stderr,"\nLetti da tastiera: dmp_fact=%.2f | err=%f | maxIt= %d | thread= %d",d,eps,maxiter,taux);

    int n_nodi = g->nodi;
    double dead_end=-1.0;
    double error=0;
    int index=n_nodi;
    double damp= d;


    pthread_t thread[taux];

    pthread_mutex_t mutex= PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t calc_mutex= PTHREAD_MUTEX_INITIALIZER;

    pthread_cond_t can_update = PTHREAD_COND_INITIALIZER;
    pthread_cond_t free= PTHREAD_COND_INITIALIZER;

    int endedt=n_nodi;


    double *x= malloc(sizeof(double)*n_nodi);
    double *y= malloc(sizeof(double)*n_nodi);
    double *next= malloc(sizeof(double)*n_nodi);

    //carico il vettore iniziale
    for(int j=0;j<n_nodi;j++){
        x[j]=(double)1/n_nodi;
        next[j]=0.0;
    }

    dati data[n_nodi];

    for(int i=0;i<taux;i++){
        data[i].calc_mutex=&calc_mutex;
        data[i].ended=&endedt;
        data[i].can_update= &can_update;
        data[i].damp=d;
        data[i].g= g;
        data[i].dead_end=&dead_end;
        data[i].free=&free;
        data[i].index= &index;
        data[i].NEXT= next;
        data[i].index_mutex=&mutex;
        data[i].error=&error;
        data[i].X = x;
        data[i].Y = y;
        xpthread_create(&thread[i],NULL,thread_job,&data[i],pos);
    }

    int iterazioni=0;
    while(iterazioni<maxiter){//produce indici di X[i] su cui devono lavorare i thread
        xpthread_mutex_lock(&mutex,pos);
        fprintf(stderr,"\nProd wait %d %d",endedt,index);
        while(endedt!=n_nodi){ pthread_cond_wait(&can_update,&mutex);}
        fprintf(stderr,"\nAggiorno valori");
        endedt=0;
        index=0;
        dead_end=-1.0;
        for(int i=0;i<n_nodi && iterazioni!=0;i++){ x[i]=next[i]; }
        iterazioni++;
        pthread_cond_broadcast(&free);
        xpthread_mutex_unlock(&mutex,pos);
    } 

    for(int i=0;i<taux;i++){
        xpthread_mutex_lock(&mutex,pos);
        while(endedt!=n_nodi) pthread_cond_wait(&can_update,&mutex);
        index=EOF; //term value
        pthread_cond_signal(&free);
        xpthread_mutex_unlock(&mutex,pos);
    }

    for(int j=0;j<taux;j++){
        xpthread_join(thread[j],NULL,pos);
    }
    double s=0.0;

    for(int j=0;j<n_nodi;j++){
        fprintf(stderr,"\n[%d]: %f",j,next[j]);
        s+=next[j];
    }

    fprintf(stderr,"\nWith sum of %f",s);
    return NULL;
}
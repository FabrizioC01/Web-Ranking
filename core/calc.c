#include "graph.h"

#define pos __LINE__,__FILE__

typedef struct calc{
    graph *g;
    double damp;
    double teleporting;
    double *dead_sum;
    int *error;
    double *X;
    double *Y;
    double *NEXT;
    pthread_mutex_t *vectors;
    //utili a capire che indice sta calcolando
    int *index;
    pthread_mutex_t *index_mutex;
    pthread_cond_t *free;
    pthread_cond_t *done;
} dati;

void hard_job(graph *g, double *X,double *Y){
    double tmp=0;
    fprintf(stderr,"\nPopolazione di Y:");
    for(int i=0;i<g->nodi;i++){
        if(g->out[i]>0){
            Y[i]=X[i]/g->out[i];
        }else{
            Y[i]=0.0;
        }
        fprintf(stderr,"[%d]:%f",i,Y[i]);
    }
}

void *thread_job(void *data){
    dati *d= (dati *)data;
    int ind;
    while(true){
        xpthread_mutex_lock(d->index_mutex,pos);
        while(*(d->index)==-1){
            pthread_cond_wait(d->free,d->index_mutex);
        }
        if(*(d->index)!=-2){
            //fprintf(stderr,"\nUn thread  ha letto %d",*d->index);
            ind=*d->index;
            *(d->index)-=1;
        }else{
            //fprintf(stderr,"\nUn thread sta terminando...");
            pthread_cond_signal(d->done);
            xpthread_mutex_unlock(d->index_mutex,pos);
            break;
        }
        pthread_cond_signal(d->done);//forse da spostare piu avanti

        xpthread_mutex_unlock(d->index_mutex,pos);


        xpthread_mutex_lock(d->vectors,pos);

        if(ind!=-1 && ind!=-2){
            if(*d->dead_sum==-1.0){ //Eseguito ogni iterazione il calcolo del contributo dei node DE
                *d->dead_sum=0.0;
                for(int i=0;i<d->g->nodi;i++){
                    if(d->g->out[i]==0){
                        *d->dead_sum+=d->X[i];
                    }
                }
                *d->dead_sum*=d->damp/(d->g->nodi);
                //fprintf(stderr,"\nDead Sum aggiornato a %f dai nodi %d",*d->dead_sum,d->g->nodi);
            }
            //devo popolare Y
            hard_job(d->g,d->X,d->Y);
            fprintf(stderr,"\n\n\nUn thread sta calcolando %d esimo elemento\n",ind);
            inmap *head=d->g->in[ind];
            double res=0.0;
            fprintf(stderr,"\nLeggo la lista di %d: ",ind);
            while(head!=NULL){
                //fprintf(stderr,"\n then Y[%d]: %f",head->value,d->Y[head->value]);
                res=(res)+d->Y[head->value];
                head=head->next;
            }
            res=(d->damp)*(res);
            fprintf(stderr,"\nSum of inmap nodes: %f \n",res);
            d->NEXT[ind]=(double)(d->teleporting)+(*d->dead_sum)+(res);
            fprintf(stderr,"\n\n\nUn thread sta terminando il calcolo del %d esimo elemento\n",ind);

            xpthread_mutex_unlock(d->vectors,pos);
        }
    }
    pthread_exit(NULL);
}

double *pagerank(graph *g, double d, double eps, int maxiter, int taux, int *numiter){
    int n_nodi = g->nodi;
    pthread_t thread[taux];
    double dead_sum=-1.0;
    int error=0;
    int index=n_nodi-1;
    pthread_mutex_t mutex= PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t data_mutex= PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t free= PTHREAD_COND_INITIALIZER;
    pthread_cond_t done= PTHREAD_COND_INITIALIZER;

    const double teleporting = (1-d)/(n_nodi);

    double *x= malloc(sizeof(double)*n_nodi);
    double *y= malloc(sizeof(double)*n_nodi);
    double *next= malloc(sizeof(double)*n_nodi);

    //carico il vettore iniziale
    for(int j=0;j<n_nodi;j++){
        x[j]=(double)1/(n_nodi);
    }

    dati data[n_nodi];

    for(int i=0;i<taux;i++){
        data[i].vectors=&data_mutex;
        data[i].damp=d;
        data[i].g= g;
        data[i].dead_sum=&dead_sum;
        data[i].done=&done;
        data[i].free=&free;
        data[i].index= &index;
        data[i].NEXT= next;
        data[i].index_mutex=&mutex;
        data[i].teleporting=teleporting;
        data[i].error=&error;
        data[i].X = x;
        data[i].Y = y;
        xpthread_create(&thread[i],NULL,thread_job,&data[i],pos);
    }

    int i=0;

    while(true && i<maxiter){//produce indici di X[i] su cui devono lavorare i thread
        xpthread_mutex_lock(&mutex,pos);
        while(index!=-1){
            pthread_cond_wait(&done,&mutex);
        }
        //qui devo mettere next in X - questo succede alla fine dell'iterazione
        xpthread_mutex_lock(&data_mutex,pos);
        fprintf(stderr,"\nSwapping puntatori X <-> NEXT");
        *x=*next;
        xpthread_mutex_unlock(&data_mutex,pos);
        fprintf(stderr,"\nSto aggiornando gli indici..");
        dead_sum=-1;
        index=n_nodi-1;
        i++;
        pthread_cond_broadcast(&free);
        xpthread_mutex_unlock(&mutex,pos);
    } //quando ha terminato 

    int ended_threads=0;

    while(ended_threads!=taux-1){
        xpthread_mutex_lock(&mutex,pos);
        index=-2;
        ended_threads++;
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
    fprintf(stderr,"\nSum of ranks: %f\n",s);
    return NULL;
}
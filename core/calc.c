#include "calc.h"

#define pos __LINE__,__FILE__


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
            xwrite(STDERR_FILENO,"\n== SIGUSR1 recived ==",23,pos);
            xpthread_mutex_lock(data->data_mutex,pos);
            ssize_t s1=sprintf(BUFF,"\nIterazione: %d",*data->iter);
            xwrite(STDERR_FILENO,BUFF,s1,pos);
            double max=DBL_MIN;
            int tmp;
            for(int i=0;i<data->s;i++){
                if(data->array[i]>max){ max=data->array[i]; tmp=i;}
            }
            xpthread_mutex_unlock(data->data_mutex,pos);
            ssize_t s2=sprintf(BUFF,"\nBest value: [%d] %f\n",tmp,max);
            xwrite(STDERR_FILENO,BUFF,s2,pos);
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
        while(*d->index==d->g->nodi) xpthread_cond_wait(d->free,d->index_mutex,pos);
        if(*d->index==EOF){
            xpthread_mutex_unlock(d->index_mutex,pos);
            break;    
        }

        ind = *d->index;
        (*d->index)+=1;
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

        if(d->g->out[ind]==0) *d->tmpDE+=d->NEXT[ind];
        else d->tmpY[ind]=d->NEXT[ind]/d->g->out[ind];
        *d->error+=fabs(d->NEXT[ind]-d->X[ind]);

        (*d->ended)+=1;

        if(*d->ended==d->g->nodi){
            *d->tmpDE*=(d->damp)/d->g->nodi;
        }
        xpthread_cond_signal(d->can_update,pos);
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
    double dead_end=0.0;
    double error= 0.0;
    int index=n_nodi;



    pthread_t thread[taux];

    pthread_mutex_t mutex= PTHREAD_MUTEX_INITIALIZER;

    pthread_cond_t can_update = PTHREAD_COND_INITIALIZER;
    pthread_cond_t lib= PTHREAD_COND_INITIALIZER;

    int endedt=n_nodi;


    double *x= malloc(sizeof(double)*n_nodi);
    double *y= malloc(sizeof(double)*n_nodi);
    double *next= malloc(sizeof(double)*n_nodi);

    double tempDE= 0.0;
    double *tempY = malloc(sizeof(double)*n_nodi);

    //carico i dati iniziali
    for(int j=0;j<n_nodi;j++){
        x[j]=(double)1/n_nodi;
        if(g->out[j]>0){
            tempY[j]=x[j]/g->out[j];
        }else{
            tempDE+=x[j];
        }
    }
    tempDE*=d/g->nodi;
    
    
    dati *data = (dati *)malloc(sizeof(dati)*taux);
    signal_data s;
    s.array=next;
    s.s=n_nodi;
    s.data_mutex=&mutex;
    int iterazioni=0;
    s.iter=&iterazioni;
    xpthread_create(&signal_handler,NULL,sig_handler,&s,pos);

    for(int i=0;i<taux;i++){
        data[i].tmpDE=&tempDE;
        data[i].tmpY=tempY;
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
        while(endedt!=n_nodi){ xpthread_cond_wait(&can_update,&mutex,pos);}
        if(error<eps && iterazioni!=0){
            xpthread_mutex_unlock(&mutex,pos);
            fprintf(stdout,"\nConverged after %d iterations",iterazioni);
            break;
        }
        endedt=0;
        index=0;
        dead_end=tempDE;
        error=0.0;
        tempDE=0.0;
        for(int i=0;i<n_nodi;i++){ 
            if(iterazioni!=0){
                x[i]=next[i]; 
            }
            y[i]=tempY[i];
        }
        iterazioni++;
        xpthread_cond_broadcast(&lib,pos);
        xpthread_mutex_unlock(&mutex,pos);
    } 
    if(!(iterazioni<maxiter)){
        fprintf(stdout,"\nDid not converge after %d iterations",iterazioni);
    }
    for(int i=0;i<taux;i++){
        xpthread_mutex_lock(&mutex,pos);
        while(endedt!=n_nodi) xpthread_cond_wait(&can_update,&mutex,pos);
        index=EOF; //term value
        xpthread_cond_signal(&lib,pos);
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
    xpthread_cond_destroy(&can_update,pos);
    xpthread_cond_destroy(&lib,pos);
    free(tempY);
    free(data);
    free(x);
    free(y);
    return next;
}
#include "graph.h"
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
    double *X;
    double *Y;
    double *NEXT;
    pthread_cond_t *can_update;
    int *index;
    pthread_mutex_t *index_mutex;
    pthread_cond_t *free;
    double *tmpY;
    double *tmpDE;
} dati;

double *pagerank(graph *g, double d, double eps, int maxiter, int taux, int *numiter);
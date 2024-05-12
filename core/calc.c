#include "graph.h"


double *pagerank(graph *g, double d, double eps, int maxiter, int taux, int *numiter){
    int n_nodi = g->nodi;
    double damping_factor = (1-d)/n_nodi;

    double *x= malloc(sizeof(double)*n_nodi);
    double *y= malloc(sizeof(double)*n_nodi);
    double *next= malloc(sizeof(double)*n_nodi);

    //carico il vettore iniziale
    for(int i=0;i<n_nodi;i++){
        x[i]=1/n_nodi;
    }

}
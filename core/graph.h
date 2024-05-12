#include "../error/error.h"

//Funzione che inizia la procedura di creazione del grafo dal file "infile"
graph *graph_init(const int nodes,const int threads,const int iter,const int damp,const int m_err,FILE *infile);

void graph_delete(graph **g);

//Funzione che effettua il calcolo effettivo dei vettori (restituisce un array di double con len=num nodi)
double *pagerank(graph *g, double d, double eps, int maxiter, int taux, int *numiter);
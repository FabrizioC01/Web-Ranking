#include "core/graph.h"

#define pos __LINE__,__FILE__

typedef struct values{
    int index;
    double value;
}top_vals;

int cmp_func(const void *a, const void *b){
    top_vals v1 = *(top_vals *)a;
    top_vals v2 = *(top_vals *)b;

    if(v1.value>v2.value) return -1;
    if(v1.value<v2.value) return 1;
    return 0;
}

int main(int argc, char *argv[]){
    int c;
    int top_nodes=K_NODES, max_it=MAX_ITER,th=N_THREADS;
    double damp=DMP_FACT;
    double err=MAX_ERR;

    while ((c=getopt(argc,argv,"k:m:d:e:t:"))!=-1){
        switch (c){
        case 'k':
            top_nodes=atoi(optarg);
            break;
        case 'm':
            max_it=atoi(optarg);
            break;
        case 'd':
            damp=atof(optarg);
            break;
        case 'e':
            err=atof(optarg);
            break;
        case 't':
            th=atoi(optarg);
            break;
        default:
            fprintf(stderr,USAGE_MSG);
            exit(1);
            break;
        }
    }
    
    if(optind+1!=argc){
        fprintf(stderr,USAGE_MSG);
        exit(1);
    }



    FILE *fp = xfopen(argv[argc-1],"r",pos);

    graph *g = graph_init(th,fp);

    int iter=0;
    double *v = pagerank(g,damp,err,max_it,th,&iter);

    top_vals *t = malloc((g->nodi)*sizeof(top_vals));
    for(int i=0;i<g->nodi;i++) {
        t[i].value=v[i];
        t[i].index=i;
    }

    qsort(t,g->nodi,sizeof(top_vals),&cmp_func);

    for(int i=0;i<top_nodes;i++){
        fprintf(stdout,"\n  %d %f",t[i].index,t[i].value);
    }


    graph_delete(&g);
    
    
    fclose(fp);
    free(v);
    free(t);
    return 0;
}
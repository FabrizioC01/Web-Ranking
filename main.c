#include "core/graph.h"

#define pos __LINE__,__FILE__

int main(int argc, char *argv[]){
    int c;
    int top_nodes=K_NODES, max_it=MAX_ITER,th=N_THREADS;
    double damp=DMP_FACT, err=MAX_ERR;

    while ((c=getopt(argc,argv,"k:m:d:e:t:"))!=-1){
        switch (c){
        case 'k':
            top_nodes=atoi(optarg);
            break;
        
        case 'm':
            max_it=atoi(optarg);
            break;
        case 'd':
            damp=atoi(optarg);
            break;
        case 'e':
            err=atoi(optarg);
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

    graph_delete(&g);
    
    fclose(fp);
    return 0;
}
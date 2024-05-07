#include "error/error.h"
#include "core/declarations.h"
#include "core/graph.h"

#define pos __LINE__,__FILE__

int main(int argc, char *argv[]){
    int c;
    int top_nodes=K_NODES, max_it=MAX_ITER, damp=DMP_FACT, err=MAX_ERR,th=N_THREADS;

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

    start_graph(top_nodes,th,max_it,damp,err,fp);

    return 0;
}
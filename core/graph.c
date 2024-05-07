#include "graph.h"

typedef struct{
    int l;
    int r;
} pair;

typedef struct{
    pair *buffer;
    int *pointer;
} shared;

void consumer_routine(void *data){

}

void start_graph(const int nodes,const int threads,const int iter,const int damp,const int m_err, FILE *infile){
    pair buffer[BUFF_SIZE];
    int index=0;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

    char *line,*token;
    while(true){
        int e = fscanf(infile,"%s",&line);
        if(line[0]!='%'){
            token = strtok(line," ");
        }
    }
}
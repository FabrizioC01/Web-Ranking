#include "error.h"

void raise_error(const char *mex,int line, char *file){
    if(errno==0){
        fprintf(stderr,"\n\n==%d== %s (%s:%d)",getpid(),mex,file,line);
    }else{
        fprintf(stderr,"\n\n==%d== %s (%s:%d) | Errno: %s",getpid(),mex,file,line,strerror(errno));
    }
    exit(1);
}

FILE *xfopen(const char *path, const char *mode, int linea, char *file) {
  FILE *f = fopen(path,mode);
  if(f==NULL) {
    perror("Errore apertura file");
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),linea,file);
    exit(1);
  }
  return f;
}
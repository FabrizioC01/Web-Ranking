#include "error.h"

#define Buflen 100
void xperror(int en, char *msg) {
  char buf[Buflen];
  
  char *errmsg = strerror_r(en, buf, Buflen);
  if(msg!=NULL)
    fprintf(stderr,"%s: %s\n",msg, errmsg);
  else
    fprintf(stderr,"%s\n",errmsg);
}

void raise_error(const char *mex,int line, char *file){
    if(errno==0){
        fprintf(stderr,"\n\n== %d == Linea: %d, File: %s - %s\n",getpid(),line,file,mex);
    }else{
        fprintf(stderr,"\n\n== %d == Linea: %d, File: %s - %s\n %s",getpid(),line,file,mex,strerror(errno));
    }
    exit(1);
}

FILE *xfopen(const char *path, const char *mode, int linea, char *file) {
  FILE *f = fopen(path,mode);
  if(f==NULL) {
    perror("Errore apertura file");
    fprintf(stderr,"\n\n== %d == Linea: %d, File: %s\n",getpid(),linea,file);
    exit(1);
  }
  return f;
}

int xsem_init(sem_t *sem, int pshared, unsigned int value, int linea, char *file) {
  int e = sem_init(sem,pshared,value);
  if(e !=0) {
    perror("Errore sem_init"); 
    fprintf(stderr,"\n\n== %d == Linea: %d, File: %s\n",getpid(),linea,file);
    exit(1);
  }
  return e;
}

int xsem_post(sem_t *sem, int linea, char *file) {
  int e = sem_post(sem);
  if(e !=0) {
    perror("Errore sem_post"); 
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),linea,file);
    exit(1);
  }
  return e;
}

int xsem_wait(sem_t *sem, int linea, char *file) {
  int e = sem_wait(sem);
  if(e !=0) {
    perror("Errore sem_wait"); 
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),linea,file);
    exit(1);
  }
  return e;
}

int xsem_destroy(sem_t *sem, int linea, char *file) {
  int e = sem_destroy(sem);
  if(e !=0) {
    perror("Errore sem_destroy"); 
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),linea,file);
    exit(1);
  }
  return e;
}

int xpthread_create(pthread_t *thread, const pthread_attr_t *attr,
                          void *(*start_routine) (void *), void *arg, int linea, char *file) {
  int e = pthread_create(thread, attr, start_routine, arg);
  if (e!=0) {
    xperror(e, "Errore pthread_create");
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),linea,file);
    pthread_exit(NULL);
  }
  return e;                       
}
                          
int xpthread_join(pthread_t thread, void **retval, int linea, char *file) {
  int e = pthread_join(thread, retval);
  if (e!=0) {
    xperror(e, "Errore pthread_join");
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),linea,file);
    pthread_exit(NULL);
  }
  return e;
}



int xpthread_mutex_init(pthread_mutex_t *restrict mutex, const pthread_mutexattr_t *restrict attr, int linea, char *file) {
  int e = pthread_mutex_init(mutex, attr);
  if (e!=0) {
    xperror(e, "Errore pthread_mutex_init");
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),linea,file);
    pthread_exit(NULL);
  }  
  return e;
}

int xpthread_mutex_destroy(pthread_mutex_t *mutex, int linea, char *file) {
  int e = pthread_mutex_destroy(mutex);
  if (e!=0) {
    xperror(e, "Errore pthread_mutex_destroy");
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),linea,file);
    pthread_exit(NULL);
  }
  return e;
}

int xpthread_mutex_lock(pthread_mutex_t *mutex, int linea, char *file) {
  int e = pthread_mutex_lock(mutex);
  if (e!=0) {
    xperror(e, "Errore pthread_mutex_lock");
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),linea,file);
    pthread_exit(NULL);
  }
  return e;
}

int xpthread_mutex_unlock(pthread_mutex_t *mutex, int linea, char *file) {
  int e = pthread_mutex_unlock(mutex);
  if (e!=0) {
    xperror(e, "Errore pthread_mutex_unlock");
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),linea,file);
    pthread_exit(NULL);
  }
  return e;
}
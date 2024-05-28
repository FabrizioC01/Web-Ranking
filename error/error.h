#include "../core/declarations.h"

void xperror(int en, char *msg);

// Funzione di gestione deglie errori che termina il programma con "mex" di errore e stampa "line" e "file" in cui è stato chiamato.
void raise_error(const char *mex,int line, char *file);

// Funzione per apertura di file
FILE *xfopen(const char *path, const char *mode, int linea, char *file);
ssize_t xwrite(int fd, const void *buf, size_t count, int line, char *file);

//Funzioni gestione errori threads
int xpthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg, int linea, char *file);                    
int xpthread_join(pthread_t thread, void **retval, int linea, char *file);

// Funzioni gestione errori delle mutex
int xpthread_mutex_init(pthread_mutex_t *restrict mutex, const pthread_mutexattr_t *restrict attr, int linea, char *file);
int xpthread_mutex_destroy(pthread_mutex_t *mutex, int linea, char *file);
int xpthread_mutex_lock(pthread_mutex_t *mutex, int linea, char *file);
int xpthread_mutex_unlock(pthread_mutex_t *mutex, int linea, char *file);

//Gestione errori condition variables
int xpthread_cond_destroy(pthread_cond_t *cond, int linea, char *file);
int xpthread_cond_wait(pthread_cond_t *restrict cond, pthread_mutex_t *restrict mutex, int linea, char *file);
int xpthread_cond_signal(pthread_cond_t *cond, int linea, char *file);
int xpthread_cond_broadcast(pthread_cond_t *cond, int linea, char *file);
#include "../core/declarations.h"

// Funzione di gestione deglie errori che termina il programma con "mex" di errore e stampa "line" e "file" in cui è stato chiamato.
void raise_error(const char *mex,int line, char *file);

// Funzione per apertura di file
FILE *xfopen(const char *path, const char *mode, int linea, char *file);
#include "directorios.h"
#include <sys/wait.h>
#include <signal.h>

struct REGISTRO { //sizeof(struct REGISTRO): 24 bytes
   time_t fecha; //Precisión segundos
   pid_t pid; //PID del proceso que lo ha creado
   int nEscritura; //Entero con el número de escritura, de 1 a 50 (orden por tiempo)
   int nRegistro; //Entero con el número del registro dentro del fichero (orden por posición)
};

void reaper();

unsigned int NUMPROCESOS = 100;
unsigned int NUMESCRITURAS = 50;
unsigned int REGMAX = 500000;
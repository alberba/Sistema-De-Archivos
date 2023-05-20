#include "simulacion.h"

#define PRUEBA 0

int acabados = 0;

int main(int argc, char **argv) {

    // Asociar la señal SIGCHLD al enterrador

    signal(SIGCHLD, reaper);

    if (argc != 2) {
        fprintf(stderr, "Error de sintaxis: ./simulacion <disco>\n");
        return FALLO;
    }

    if (bmount(argv[1]) == FALLO) {
        fprintf(stderr, "Error de montaje del disco\n");
        return FALLO;
    }

    // Generamos el nombre del archivo con la fecha y hora actual

    char camino[22] = "/simul_";

    time_t tiempo;
    time(&tiempo);
    struct tm *tm = localtime(&tiempo);

    char hora[16];
    strftime(hora, sizeof(hora), "%Y%m%d%H%M%S/", tm);
    // Concatenamos el nombre del archivo con la fecha actual
    strcat(camino, hora);


    // Creamos el directorio o fichero
    if(mi_creat(camino, 6) == FALLO) {
        fprintf(stderr, "Error de creación del directorio\n");
        return FALLO;
    }
    
    pid_t pid;
    for (int proceso = 1; proceso <= NUMPROCESOS; proceso++) {
        pid = fork();
        if (pid == 0) {
            // No es necesario el control de errores ya que sería redundante
            bmount(argv[1]);
            char directorioHijo[35];
            char *textoPid = malloc(13);
            sprintf(textoPid, "proceso_%d/", getpid());
            strcpy(directorioHijo, camino);
            strcat(directorioHijo, textoPid);
            if (mi_creat(directorioHijo, 6) == FALLO) {
                fprintf(stderr, "Error de creación del directorio\n");
                return FALLO;
            }
            char archivoHijo[38];
            
            // Restauramos el camino para llegar al directorio y añadir el nombre del archivo
            strcpy(archivoHijo, directorioHijo);
            strcat(archivoHijo, "prueba.dat");
            
            // Creamos el archivo
            if (mi_creat(archivoHijo, 7) == FALLO) {
                fprintf(stderr, "Error de creación del archivo\n");
                return FALLO;
            }

#if PRUEBA
            fprintf(stderr, "\nNombre de la ruta: %s\n", archivoHijo);
#endif

            srand(time(NULL) + getpid());
                        
            for (int nEscritura = 1; nEscritura <= NUMESCRITURAS; nEscritura++) {
                // Creamos el registro
                struct REGISTRO registro;
                
                // Asignamos los valores al registro
                registro.fecha = time(NULL);
                registro.pid = getpid();
                registro.nEscritura = nEscritura;
                registro.nRegistro = rand() % REGMAX; // [0, 499.999]
                mi_write(archivoHijo, &registro, registro.nRegistro * sizeof(struct REGISTRO), sizeof(struct REGISTRO));
#if PRUEBA
                fprintf(stderr, "[simulación.c → Escritura %i en %s]\n", nEscritura, archivoHijo);
                
#endif
                usleep(50000); // Esperar 0,05s para hacer la siguiente escritura
            }
#if DEBUGN12
            fprintf(stderr, "[Proceso %d: Completadas %d escrituras en %s]\n", proceso, NUMESCRITURAS, archivoHijo);
#endif
            
            bumount();
            exit(0);
            
        }
        usleep(15000); // Esperar 0,15s para lanzar el siguiente proceso
    }

    // Permitir que el padre espere por todos los hijos
    while (acabados < NUMPROCESOS) {
        pause();
    }

    bumount();
#if DEBUGN12
    fprintf(stderr, "Total de procesos terminados: %d\n", acabados);
#endif
    exit(0);
}

void reaper(){
    pid_t ended;
  
    signal(SIGCHLD, reaper);

    while ((ended = waitpid(-1, NULL, WNOHANG))>0) {
        acabados++;

#if PRUEBA
        fprintf(stderr, "[simulación.c → Acabado proceso con PID %d, total acabados: %d\n", ended, acabados);
#endif
    }
  
}
void my_sleep(unsigned msec) { //recibe tiempo en milisegundos
   struct timespec req, rem;
   int err;
   req.tv_sec = msec / 1000; //conversión a segundos
   req.tv_nsec = (msec % 1000) * 1000000; //conversión a nanosegundos
   while ((req.tv_sec != 0) || (req.tv_nsec != 0)) {
       if (nanosleep(&req, &rem) == 0) 
        // rem almacena el tiempo restante si una llamada al sistema
        // ha sido interrumpida por una señal
           break;
       err = errno;
       // Interrupted; continue
       if (err == EINTR) {
           req.tv_sec = rem.tv_sec;
           req.tv_nsec = rem.tv_nsec;
       }
   }
}

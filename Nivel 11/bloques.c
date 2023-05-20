#include "bloques.h"

static int descriptor = 0;
static sem_t *mutex;
static unsigned int inside_sc = 0;


int bmount(const char *camino) {
    umask(000);

    if (!mutex) { // el semáforo es único en el sistema y sólo se ha de inicializar 1 vez (padre)
       mutex = initSem(); 
       if (mutex == SEM_FAILED) {
           return FALLO;
       }
    }

    // Abre el archivo especificado
    descriptor = open(camino, O_RDWR | O_CREAT, 0666);
    return descriptor;
}

int bumount() {
    deleteSem();

    // Cierra el archivo
    return close(descriptor);
}

int bwrite(unsigned int nbloque, const void *buf) {
    off_t desplazamiento = nbloque * BLOCKSIZE;

    // Reposiciona el desplazamiento del archivo
    lseek(descriptor, desplazamiento, SEEK_SET);
    // Escribe para contar bytes
    size_t bytes = write(descriptor, buf, BLOCKSIZE);
    // Se devuelven los bytes escritos, en caso de fallo devuelve -1
    return bytes;
}

int bread(unsigned int nbloque, void *buf){
    off_t desplazamiento = nbloque * BLOCKSIZE;
    // Reposiciona el desplazamiento del archivo
    lseek(descriptor, desplazamiento, SEEK_SET);
    // Escribe para contar bytes
    size_t bytes = read(descriptor, buf, BLOCKSIZE);
    // Se devuelven los bytes leídos, en caso de fallo devuelve -1
    return bytes;
}

void mi_waitSem() {
    if (!inside_sc) { // inside_sc==0
        waitSem(mutex);
    }
    inside_sc++;
    waitSem(mutex);
}

void mi_signalSem() {
    inside_sc--;
    if (!inside_sc) {
        signalSem(mutex);
    }

}


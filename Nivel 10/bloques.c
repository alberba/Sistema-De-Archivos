// Autores: Santiago Rattenbach, Sergi Oliver y Albert Salom

#include "bloques.h"

static int descriptor = 0;


int bmount(const char *camino) {
    umask(000);
    // Abre el archivo especificado
    descriptor = open(camino, O_RDWR | O_CREAT, 0666);
    return descriptor;
}

int bumount() {
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
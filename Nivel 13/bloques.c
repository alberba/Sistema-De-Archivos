#include "bloques.h"

static int descriptor = 0;
static sem_t *mutex;
static unsigned int inside_sc = 0;
// El tamaño de la mapeado de memoria
static int tamSFM;
static void *ptrSFM;


int bmount(const char *camino) {
    if (descriptor > 0) {
       close(descriptor);
    }

    umask(000);

    if (!mutex) { // el semáforo es único en el sistema y sólo se ha de inicializar 1 vez (padre)
       mutex = initSem(); 
       if (mutex == SEM_FAILED) {
           return FALLO;
       }
    }

    // Abre el archivo especificado
    descriptor = open(camino, O_RDWR | O_CREAT, 0666);

    ptrSFM = do_mmap(descriptor);
    return descriptor;
}

int bumount() {
    msync(ptrSFM, tamSFM, MS_SYNC);
    munmap(ptrSFM, tamSFM);
    descriptor = close(descriptor);
    deleteSem();

    // Cierra el archivo
    return close(descriptor);
}

int bwrite(unsigned int nbloque, const void *buf) {
    size_t bytes;
    off_t desplazamiento = nbloque * BLOCKSIZE;
    
    if (desplazamiento + BLOCKSIZE <= tamSFM) {
        bytes = BLOCKSIZE;
    } else {
        bytes = tamSFM - desplazamiento;
    }
    if (bytes > 0) {
        memcpy(ptrSFM + desplazamiento, buf, bytes);
    }

    return bytes;

    /*off_t desplazamiento = nbloque * BLOCKSIZE;

    // Reposiciona el desplazamiento del archivo
    lseek(descriptor, desplazamiento, SEEK_SET);
    // Escribe para contar bytes
    size_t bytes = write(descriptor, buf, BLOCKSIZE);
    // Se devuelven los bytes escritos, en caso de fallo devuelve -1
    return bytes;*/
}

int bread(unsigned int nbloque, void *buf) {
    size_t bytes;
    off_t desplazamiento = nbloque * BLOCKSIZE;
    if (nbloque*BLOCKSIZE + BLOCKSIZE <= tamSFM) {
        bytes = BLOCKSIZE;
    } else {
        bytes = tamSFM - desplazamiento;
    }
    if (bytes > 0) {
        memcpy(buf, ptrSFM + desplazamiento, bytes);
    }

    return bytes;
    /*
    off_t desplazamiento = nbloque * BLOCKSIZE;
    // Reposiciona el desplazamiento del archivo
    lseek(descriptor, desplazamiento, SEEK_SET);
    // Escribe para contar bytes
    size_t bytes = read(descriptor, buf, BLOCKSIZE);
    // Se devuelven los bytes leídos, en caso de fallo devuelve -1
    return bytes;
    */
}

void mi_waitSem() {
    if (!inside_sc) { // inside_sc==0
        waitSem(mutex);
    }
    inside_sc++;
}

void mi_signalSem() {
    inside_sc--;
    if (!inside_sc) {
        signalSem(mutex);
    }

}

void *do_mmap(int fd) {
    struct stat st;
    void *ptr;
    fstat(fd, &st);
    tamSFM = st.st_size; //static int tamSFM: tamaño memoria compartida
    if ((ptr = mmap(NULL, tamSFM, PROT_WRITE, MAP_SHARED, fd, 0))== (void *)-1)
        fprintf(stderr, "Error al mapear el archivo\n"); 
    return ptr;
}



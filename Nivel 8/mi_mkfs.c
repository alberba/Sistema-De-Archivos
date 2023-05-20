#include "ficheros_basico.h"

unsigned char buf[BLOCKSIZE];

int main(int argc, char **argv) {
    char *nombreDisco = argv[1];
    // Montamos el dispositivo virtual
    int descriptor = bmount(nombreDisco);
    int nBloques = atoi(argv[2]);
    if (descriptor == FALLO){
        printf("HAY FALLO");
        return FALLO;
    }
    //Llenamos el buffer de 0s
    memset(buf, 0, BLOCKSIZE);
    for (int i = 0; i < nBloques; i++) {
        if (bwrite(i, buf) == FALLO) {
            return FALLO;
        }
    }

    //iniciamos el SuperBloque, el mapa de bits y el array de inodos
    if(initSB(nBloques,nBloques / 4) == FALLO){
        fprintf(stderr, "ERROR GENERANDO EL SUPERBLOQUE EN EL DISPOSITIVO VIRTUAL.\n");
        return FALLO;
    }
    if (initMB() == FALLO){
        fprintf(stderr, "Error en la generación del mapa de bits del dispositivo virtual.\n");
        return FALLO;
    }
    if (initAI() == FALLO)
    {
        fprintf(stderr, "Error en la generación el array de inodos del dispositivo.\n");
        return FALLO;
    }

    reservar_inodo('d',7);
    // Desmontamos el dispositivo virtual
    bumount();
}
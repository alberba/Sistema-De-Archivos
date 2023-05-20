#include "ficheros_basico.h"

unsigned char buf[BLOCKSIZE];

int main(int argc, char **argv) {
    char *nombreDisco = argv[1];
    // Montamos el dispositivo virtual
    int descriptor = bmount(nombreDisco);
    int nBloques = atoi(argv[2]);
    if (descriptor == FALLO){
        // Control de errores
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
    initSB(nBloques,nBloques / 4);
    initMB();
    initAI();

    // Desmontamos el dispositivo virtual
    bumount();
}
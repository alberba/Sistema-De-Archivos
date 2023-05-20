#include "ficheros.h"   

int main(int argc, char **argv) {
    // Revisión de sintaxis
    if (argc != 3) {
        fprintf(stderr, "Error de sintaxis\n");
        return FALLO;
    }

    int ninodo = atoi(argv[2]);
    struct superbloque SB;
    struct inodo inodo;
 
    // Se monta el dispositivo virtual
    if (bmount(argv[1]) == FALLO) {
        fprintf(stderr, "Error al montar el dispositivo virtual\n");
        return FALLO;
    }

    // Lectura del contenido del superbloque
    if (bread(posSB, &SB) == FALLO) {
        // Control de errores
        return FALLO;
    }
    
    int offset = 0;
    int nbytes = 1500;
    int bytesLeidos = 0;
    char buffer[nbytes];

    // Se inicializa el buffer
    memset(buffer, 0, nbytes);
    // Se lee el fichero
    int bytesLeidosTemp = mi_read_f(ninodo, buffer, offset, nbytes);
    // Se lee de nuevo hasta haber leído el fichero completo
    while (bytesLeidosTemp > 0) {
        // Se actualiza el nº de bytes leidos
        bytesLeidos += bytesLeidosTemp;
        // Escritura del contenido del buffer
        write(1, buffer, bytesLeidosTemp);
        // Se reinicia el contenido del buffer
        memset(buffer, 0, nbytes);
        // Se actualiza el offset para leer los siguientes datos
        offset += nbytes;
        // Se resume la lectura
        bytesLeidosTemp = mi_read_f(ninodo, buffer, offset, nbytes);
    }

    if (leer_inodo(ninodo, &inodo) == FALLO) {
        return FALLO;
    }

    fprintf(stderr,"\ntotal_bytesLeidos: %d\nTamEnBytesLog: %d\n", bytesLeidos, inodo.tamEnBytesLog);

    // Finalmente se desmonta el dispositivo virtual
    if (bumount() == FALLO)
    {
        fprintf(stderr, "Error al desmontar el dispositivo virtual\n");
        return FALLO;
    }
    return EXITO;
}
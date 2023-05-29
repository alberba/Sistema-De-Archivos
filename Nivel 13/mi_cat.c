// Autores: Santiago Rattenbach, Sergi Oliver y Albert Salom

#include "directorios.h"

int main(int argc, char **argv){
    // Comprobamos sintaxis
    if(argc != 3){
        fprintf(stderr, "Error de sintaxis\n");
        return FALLO;
    }

    if (argv[2][strlen(argv[2])-1] == '/') {
        fprintf(stderr, "No se puede leer un fichero\n");
        return FALLO;
    }

    // Montamos el disco virtual
    if(bmount(argv[1]) == FALLO){
        fprintf(stderr, "Error de montaje\n");
        return FALLO;
    }

    int tamBuffer = BLOCKSIZE * 2;
    int bytes_leidos = 0;
    int offset = 0;
    char buffer[tamBuffer];

    memset(buffer, 0, tamBuffer);

    // Creamos el directorio o fichero
    int bytesLeidosRead = mi_read(argv[2], buffer, offset, tamBuffer);

    while (bytesLeidosRead > 0){
        bytes_leidos += bytesLeidosRead;
        offset += bytesLeidosRead;

        write(1, buffer, bytesLeidosRead);

        memset(buffer, 0, tamBuffer);

        bytesLeidosRead = mi_read(argv[2], buffer, offset, tamBuffer);
    }
    fprintf(stderr, "\nTotal bytes leidos: %d\n", bytes_leidos);

    // Desmontamos el disco virtual
    if(bumount() == FALLO) {
        fprintf(stderr, "Error de desmontaje\n");
        return FALLO;
    }

}
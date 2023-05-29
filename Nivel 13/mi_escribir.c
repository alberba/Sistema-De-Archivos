// Autores: Santiago Rattenbach, Sergi Oliver y Albert Salom

#include "directorios.h"

int main(int argc, char **argv){
    // Comprobamos sintaxis
    if(argc != 5){
        fprintf(stderr, "Error de sintaxis\n");
        return FALLO;
    }

    // Montamos el disco virtual
    if(bmount(argv[1]) == FALLO){
        fprintf(stderr, "Error de montaje\n");
        return FALLO;
    }
    
    printf("Total longitud: %ld\n", strlen(argv[3]));

    // Creamos el directorio o fichero
    int bytesEscritos = mi_write(argv[2], argv[3], atoi(argv[4]), strlen(argv[3]));

    if(bytesEscritos == FALLO){
        printf("Bytes escritos: 0\n");
    } else {
        printf("Total bytes escritos: %d\n", bytesEscritos);
    }

    // Desmontamos el disco virtual
    if(bumount() == FALLO) {
        fprintf(stderr, "Error de desmontaje\n");
        return FALLO;
    }

}
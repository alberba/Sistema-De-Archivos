#include "directorios.h"

int main(int argc, char **argv){
    // Comprobamos sintaxis
    if(argc != 4){
        fprintf(stderr, "Error de sintaxis\n");
        return FALLO;
    }

    // Montamos el disco virtual
    if(bmount(argv[1]) == FALLO){
        fprintf(stderr, "Error de montaje\n");
        return FALLO;
    }

    if(atoi(argv[2]) < 0 || atoi(argv[2]) > 7) {
        fprintf(stderr, "Nº permisos incorrecto\n");
        return FALLO;
    }

    // Creamos el directorio o fichero
    mi_creat(argv[3], atoi(argv[2]));

    // Desmontamos el disco virtual
    if(bumount() == FALLO) {
        fprintf(stderr, "Error de desmontaje\n");
        return FALLO;
    }

}
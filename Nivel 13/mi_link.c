// Autores: Santiago Rattenbach, Sergi Oliver y Albert Salom

#include "directorios.h"

int main(int argc, char **argv){
    // Comprobamos sintaxis
    if(argc != 4){
        fprintf(stderr, "Error de sintaxis\n");
        return FALLO;
    }

    if (argv[2][strlen(argv[2])-1] == '/') {
        fprintf(stderr, "La ruta original no es un fichero\n");
        return FALLO;
    }

    if (argv[3][strlen(argv[3])-1] == '/') {
        fprintf(stderr, "La ruta de enlace no es un fichero\n");
        return FALLO;
    }

    // Montamos el disco virtual
    if(bmount(argv[1]) == FALLO){
        fprintf(stderr, "Error de montaje\n");
        return FALLO;
    }

    mi_link(argv[2], argv[3]);

    // Desmontamos el disco virtual
    if(bumount() == FALLO) {
        fprintf(stderr, "Error de desmontaje\n");
        return FALLO;
    }

}
// Autores: Santiago Rattenbach, Sergi Oliver y Albert Salom

#include "directorios.h"

int main(int argc, char **argv){
    // Comprobamos sintaxis
    if(argc != 4){
        fprintf(stderr, "Sintaxis: ./mi_chmod <nombre_dispositivo> <permisos> </ruta>\n");
        return FALLO;
    }
    int permisos = atoi(argv[2]);

    if(permisos < 0 || permisos > 7){
        fprintf(stderr, "Error de sintaxis\n");
        return FALLO;
    }

    // Montamos el disco virtual
    if(bmount(argv[1]) == FALLO){
        fprintf(stderr, "Error de montaje\n");
        return FALLO;
    }

    if(mi_chmod(argv[3], permisos) == FALLO){
        fprintf(stderr, "Error al cambiar los permisos\n");
        return FALLO;
    }

    // Desmontamos el disco virtual
    if(bumount() == FALLO){
        fprintf(stderr, "Error de desmontaje\n");
        return FALLO;
    }

}
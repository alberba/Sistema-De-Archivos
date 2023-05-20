#include "ficheros.h"

int main(int argc, char *argv[]){
    int ninodo = atoi(argv[2]);
    int permisos = atoi(argv[3]);

    //validacion de permisos
    if (argc != 4){
        fprintf(stderr, "Sintaxis: permitir <nombre_dispositivo> <ninodo> <permisos>\n");
        return FALLO;
    }

    // Monta el dispositivo en el sistema.
    if (bmount(argv[1]) == FALLO){
        fprintf(stderr, "Error montando el dispositivo en el sistema (permitir.c).\n");
        return FALLO;
    }
    //Se cambian los permisos
    if(mi_chmod_f(ninodo, permisos) == FALLO){
        fprintf(stderr, "Error al cambiar los permisos(permitir.c).\n");
        return FALLO;
    }
    // Desmonta el dispositivo
    if (bumount() == FALLO){
        fprintf(stderr, "Error al desmonta el dispositivo (permitir.c).\n");
        return FALLO;
    }

    return EXITO;
}
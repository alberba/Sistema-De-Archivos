#include "directorios.h"

int main(int argc, char **argv){
    // Comprobamos sintaxis
    if(argc != 3){
        fprintf(stderr, "Error de sintaxis\n");
        return FALLO;
    }

    // Montamos el disco virtual
    if(bmount(argv[1]) == FALLO){
        fprintf(stderr, "Error de montaje\n");
        return FALLO;
    }
    
    char buffer[1000000];
    memset(buffer, 0, 1000000);
    int total;

    if((total = mi_dir(argv[2], buffer)) < FALLO){
        fprintf(stderr, "Error de lectura\n");
        return FALLO;
    }
    

    if(total > 0){
#if DEBUGN8
        printf("Total: %d\n", total);
#endif
        printf("Tipo\tModo\tmTime\t\t\tTamaño\tNombre\n");
        printf("--------------------------------------------------------------------------------\n");
        printf("%s\n", buffer);
    }

    // Desmontamos el disco virtual
    if(bumount() == FALLO){
        fprintf(stderr, "Error de desmontaje\n");
        return FALLO;
    }

}
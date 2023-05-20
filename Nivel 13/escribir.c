#include "ficheros.h"

int main(int argc, char **argv){
    if(argc != 4){
        fprintf(stderr, "Error de sintaxis\n");
        return 0;
    }
    int offsets[5] = {9000, 209000, 30725000, 409605000, 480000000};

    if(bmount(argv[1]) == FALLO){
        fprintf(stderr, "Error al montar el dispositivo virtual\n");
        return FALLO;
    }
    

    int ninodo = reservar_inodo('f',6);

    if(ninodo == -1){
        fprintf(stderr, "Error al reservar el inodo\n");
        return FALLO;
    }

    for(int i = 0; i < (sizeof(offsets) / sizeof(int)); i++){
        unsigned int longitud = strlen(argv[2]);
        int numBytes = mi_write_f(ninodo, argv[2], offsets[i], longitud);
        if(numBytes == FALLO){
            fprintf(stderr, "Error al escribir");
            return FALLO;
        }
        printf("Bytes escritos: %d\n",numBytes);
        char *auxBuffer [longitud];
        memset(auxBuffer, 0, longitud);

        struct STAT p_stat;
        if(mi_stat_f(ninodo, &p_stat) == FALLO){
            fprintf(stderr, "Error en el metodo mi_stat_f\n");
            return FALLO;
        }
        printf("Tamaño en bytes lógico del inodo: %u\n", p_stat.tamEnBytesLog);
        printf("Número de bloques ocupados: %u\n\n", p_stat.numBloquesOcupados);

        if(strcmp(argv[3], "0")){
            ninodo = reservar_inodo('f', 6);
            if(ninodo == -1){
                fprintf(stderr, "Error al reservar el inodo\n");
                return FALLO;
            }
        }
        
    }
    if(bumount() == FALLO){
        fprintf(stderr, "Error al desmontar el disco");
        return FALLO;
    }

}
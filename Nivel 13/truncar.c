// Autores: Santiago Rattenbach, Sergi Oliver y Albert Salom

#include "ficheros.h"

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "ERROR DE SINTAXIS\n");
        return FALLO;
    }

    if (bmount(argv[1]) == FALLO) {
        fprintf(stderr, "Error al montar el dispositivo virtual\n");
        return FALLO;
    }
    unsigned int nbytes = atoi(argv[3]);
    unsigned int ninodo = atoi(argv[2]);


    if (nbytes == 0) {
        if (liberar_inodo(ninodo) == FALLO) {
            fprintf(stderr, "Error al liberar el inodo");
            return FALLO;
        }
    } else {
        mi_truncar_f(ninodo, nbytes);
    }

    //Inodo liberado
    struct STAT p_stat;
    if (mi_stat_f(ninodo, &p_stat) == FALLO) {
        fprintf(stderr, "truncar.c: Error mi_stat_f()\n");
        return FALLO;
    }
    
    // Variables utilizadas para cambiar el formato de la fecha y hora.
    struct tm *ts;
    char atime[80];
    char mtime[80];
    char ctime[80];
    ts = localtime(&p_stat.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&p_stat.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&p_stat.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);

    //Información del inodo escrito
    printf("\nDATOS INODO %d:\n", ninodo);
    printf("tipo=%c\n", p_stat.tipo);
    printf("permisos=%d\n", p_stat.permisos);
    printf("atime: %s\n", atime);
    printf("ctime: %s\n", ctime);
    printf("mtime: %s\n", mtime);
    printf("nLinks= %d\n", p_stat.nlinks);
    printf("tamEnBytesLog= %d\n", p_stat.tamEnBytesLog);
    printf("numBloquesOcupados= %d\n", p_stat.numBloquesOcupados);

    if (bumount(argv[1]) == FALLO) {
        fprintf(stderr, "Error al desmontar el dispositivo virtual\n");
        return FALLO;
    }
}
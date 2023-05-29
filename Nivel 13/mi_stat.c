// Autores: Santiago Rattenbach, Sergi Oliver y Albert Salom

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
    struct STAT p_stat;
    int p_inodo = mi_stat(argv[2], &p_stat);

    if(p_inodo == FALLO){
        fprintf(stderr, "Error al cambiar los permisos\n");
        return FALLO;
    }

    // Desmontamos el disco virtual
    if(bumount() == FALLO){
        fprintf(stderr, "Error de desmontaje\n");
        return FALLO;
    }

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

    printf("Nº inodo: %d\n", p_inodo);
    printf("Tipo: %c\n", p_stat.tipo);
    printf("Permisos: %d\n", p_stat.permisos);
    printf("Atime: %s\n", atime);
    printf("Ctime: %s\n", ctime);
    printf("Mtime: %s\n", mtime);
    printf("Nlinks: %d\n", p_stat.nlinks);
    printf("tamEnBytesLog: %d\n", p_stat.tamEnBytesLog);
    printf("nBloquesOcupados: %d\n", p_stat.numBloquesOcupados);

}
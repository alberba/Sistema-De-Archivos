#include "ficheros_basico.h"

int main(int argc, char **argv) {
    struct superbloque SB;
    struct inodo inodos[BLOCKSIZE/INODOSIZE];
    char *nombreDisco = argv[1];
    // Montamos el disco pasado por parámetro
    if (bmount(nombreDisco) == FALLO) {
        // Control de errores
        fprintf(stderr, "No se ha podido obtener el file descriptor.\n");
        return FALLO;
    }

    if (bread(posSB, &SB) == FALLO) {
        return FALLO;
    }

    // Muestra por pantalla del contenido del SuperBloque
    
    printf("posPrimerBloqueMB struct superbloque is: %u\n", SB.posPrimerBloqueMB);
    printf("posUltimoBloqueMB struct superbloque is: %u\n", SB.posUltimoBloqueMB);
    printf("posPrimerBloqueAI struct superbloque is: %u\n", SB.posPrimerBloqueAI);
    printf("posUltimoBloqueAI struct superbloque is: %u\n", SB.posUltimoBloqueAI);
    printf("posPrimerBloqueDatos struct superbloque is: %u\n", SB.posPrimerBloqueDatos);
    printf("posUltimoBloqueDatos struct superbloque is: %u\n", SB.posUltimoBloqueDatos);
    printf("posInodoRaiz struct superbloque is: %u\n", SB.posInodoRaiz);
    printf("posPrimerInodoLibre struct superbloque is: %u\n", SB.posPrimerInodoLibre);
    printf("cantBloquesLibres struct superbloque is: %u\n", SB.cantBloquesLibres);
    printf("cantInodosLibres struct superbloque is: %u\n", SB.cantInodosLibres);
    printf("totBloques struct superbloque is: %u\n", SB.totBloques);
    printf("totInodos struct superbloque is: %u\n", SB.totInodos);

    printf ("sizeof struct inodo is: %lu\n", sizeof(struct inodo));
    printf("\nsizeof struct superbloque is: %lu\n", sizeof(struct superbloque));

    // Muestra por pantalla de los distintos inodos libres que existen dentro del dispositivo
    printf("RECORRIDO LISTA ENLAZADA DE INODOS LIBRES\n");
    for (size_t i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++) {
        bread(i, inodos);
        for (size_t j = 0; j < (BLOCKSIZE / INODOSIZE); j++){
            if(inodos[j].tipo == 'l'){
                printf("%d ", inodos[j].punterosDirectos[0]);
            }
        }
    }

    // Desmontamos el dispositivo virtual
    bumount();

    return 0;
}
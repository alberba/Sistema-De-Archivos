#include "ficheros_basico.h"

int main(int argc, char **argv) {
    struct superbloque SB;
    char *nombreDisco = "disco";
    //char *nombreDisco = argv[1];
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

    // Muestra del primer y último bit del MB de cada zona del disco
    printf("MAPA DE BITS CON BLOQUES DE METADATOS OCUPADOS\n");
    printf("posSB: %d -> leer_bit(%d) = %d\n\n", posSB, posSB, leer_bit(posSB));
    printf("SB.posPrimerBloqueMB: %u -> leer_bit(%u) = %d\n\n", SB.posPrimerBloqueMB, SB.posPrimerBloqueMB, leer_bit(SB.posPrimerBloqueMB));
    printf("SB.posUltimoBloqueMB: %u -> leer_bit(%u) = %d\n\n", SB.posUltimoBloqueMB, SB.posUltimoBloqueMB, leer_bit(SB.posUltimoBloqueMB));
    printf("SB.posPrimerBloqueAI: %u -> leer_bit(%u) = %d\n\n", SB.posPrimerBloqueAI, SB.posPrimerBloqueAI, leer_bit(SB.posPrimerBloqueAI));
    printf("SB.posUltimoBloqueAI: %u -> leer_bit(%u) = %d\n\n", SB.posUltimoBloqueAI, SB.posUltimoBloqueAI, leer_bit(SB.posUltimoBloqueAI));
    printf("SB.posPrimerBloqueDatos: %u -> leer_bit(%u) = %d\n\n", SB.posPrimerBloqueDatos, SB.posPrimerBloqueDatos, leer_bit(SB.posPrimerBloqueDatos));
    printf("SB.posUltimoBloqueDatos: %u -> leer_bit(%u) = %d\n\n", SB.posUltimoBloqueDatos, SB.posUltimoBloqueDatos, leer_bit(SB.posUltimoBloqueDatos));

#if DEBUG1
    // Comprobación de reserva y liberación de un inodo
    printf("RESERVAMOS UN BLOQUE Y LUEGO LO LIBERAMOS\n");
    // Reservamos el primer bloque disponible
    int bloque_reservado = reservar_bloque();
    printf("Se ha reservado el bloque físico nº%d que era el 1º libre indicado por el MB\n", bloque_reservado);
    if (bread(posSB, &SB) == FALLO) {
        return FALLO;
    }
    printf("SB.cantBloquesLibres = %d\n", SB.cantBloquesLibres);
    // Liberamos el bloque anterior
    liberar_bloque(bloque_reservado);
    if (bread(posSB, &SB) == FALLO) {
        return FALLO;
    }
    printf("Liberamos ese bloque y después SB.cantBloquesLibres = %u\n", SB.cantBloquesLibres);
#endif
    struct tm *ts;
    char atime[80];
    char mtime[80];
    char ctime[80];
    
    struct inodo inodo;
    int ninodo = 0;
    leer_inodo(ninodo, &inodo);

    printf("DATOS DEL DIRECTORIO RAIZ\n");
    printf("tipo: %c\n", inodo.tipo);
    printf("permisos: %u\n", inodo.permisos);
    ts = localtime(&inodo.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
    printf("ID: %d \nATIME: %s \nMTIME: %s \nCTIME: %s\n",ninodo,atime,mtime,ctime);
    printf("nlinks: %u\n", inodo.nlinks);
    printf("tamEnBytesLog: %u\n", inodo.tamEnBytesLog);
    printf("numBloquesOcupados: %u\n", inodo.numBloquesOcupados);

    struct inodo inodoReservado;
    int num = reservar_inodo((unsigned char) 'f', 6);
    leer_inodo(num, &inodoReservado);

    traducir_bloque_inodo(&inodoReservado, 8, 1);
    printf("\n");
    traducir_bloque_inodo(&inodoReservado, 204, 1);
    printf("\n");
    traducir_bloque_inodo(&inodoReservado, 30004, 1);
    printf("\n");
    traducir_bloque_inodo(&inodoReservado, 400004, 1);
    printf("\n");
    traducir_bloque_inodo(&inodoReservado, 468750, 1);

    // Desmontamos el dispositivo virtual
    bumount();

    return 0;
}
#include "directorios.h"

#define DEBUG 0

void mostrar_buscar_entrada(char *camino, char reservar){
  unsigned int p_inodo_dir = 0;
  unsigned int p_inodo = 0;
  unsigned int p_entrada = 0;
  int error;
  printf("\ncamino: %s, reservar: %d\n", camino, reservar);
  if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, reservar, 6)) < 0) {
    mostrar_error_buscar_entrada(error);
  }
  printf("**********************************************************************\n");
  return;
}



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
#if DEBUGNSB   
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
#endif

#if DEBUGN7
    mostrar_buscar_entrada("pruebas/", 1); //ERROR_CAMINO_INCORRECTO
    mostrar_buscar_entrada("/pruebas/", 0); //ERROR_NO_EXISTE_ENTRADA_CONSULTA
    mostrar_buscar_entrada("/pruebas/docs/", 1); //ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO
    mostrar_buscar_entrada("/pruebas/", 1); // creamos /pruebas/
    mostrar_buscar_entrada("/pruebas/docs/", 1); //creamos /pruebas/docs/
    mostrar_buscar_entrada("/pruebas/docs/doc1", 1); //creamos /pruebas/docs/doc1
    mostrar_buscar_entrada("/pruebas/docs/doc1/doc11", 1);  
    //ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO
    mostrar_buscar_entrada("/pruebas/", 1); //ERROR_ENTRADA_YA_EXISTENTE
    mostrar_buscar_entrada("/pruebas/docs/doc1", 0); //consultamos /pruebas/docs/doc1
    mostrar_buscar_entrada("/pruebas/docs/doc1", 1); //creamos /pruebas/docs/doc1
    mostrar_buscar_entrada("/pruebas/casos/", 1); //creamos /pruebas/casos/
    mostrar_buscar_entrada("/pruebas/docs/doc2", 1); //creamos /pruebas/docs/doc2

#endif

#if DEBUGN2
    printf("\nsizeof struct superbloque: %ld\n", sizeof(struct superbloque));
    printf("sizeof struct inodo:  %ld\n", sizeof(struct inodo));

    printf("\nRECORRIDO LISTA ENLAZADA DE INODOS LIBRES\n");
    //Podéis hacer también un recorrido de la lista de inodos libres (mostrando para cada inodo el campo punterosDirectos[0]).
    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    int contlibres = 0;

    for (int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++)
    {
        //&inodos
        if (bread(i, inodos) == FALLO)
        {
            return FALLO;
        }

        for (int j = 0; j < BLOCKSIZE / INODOSIZE; j++)
        {
            if ((inodos[j].tipo == 'l'))
            {
                contlibres++;
                if (contlibres < 20)
                {
                    printf("%d ", contlibres);
                }
                else if (contlibres == 21)
                {
                    printf("... ");
                }
                else if ((contlibres > 24990) && (contlibres < SB.totInodos))
                {
                    printf("%d ", contlibres);
                }
                else if (contlibres == SB.totInodos)
                {
                    printf("-1 \n");
                }
                contlibres--;
            }
            contlibres++;
        }
    }
#endif

#if DEBUGN3
    // Muestra del primer y último bit del MB de cada zona del disco
    printf("MAPA DE BITS CON BLOQUES DE METADATOS OCUPADOS\n");
    printf("posSB: %d -> leer_bit(%d) = %d\n\n", posSB, posSB, leer_bit(posSB));
    printf("SB.posPrimerBloqueMB: %u -> leer_bit(%u) = %d\n\n", SB.posPrimerBloqueMB, SB.posPrimerBloqueMB, leer_bit(SB.posPrimerBloqueMB));
    printf("SB.posUltimoBloqueMB: %u -> leer_bit(%u) = %d\n\n", SB.posUltimoBloqueMB, SB.posUltimoBloqueMB, leer_bit(SB.posUltimoBloqueMB));
    printf("SB.posPrimerBloqueAI: %u -> leer_bit(%u) = %d\n\n", SB.posPrimerBloqueAI, SB.posPrimerBloqueAI, leer_bit(SB.posPrimerBloqueAI));
    printf("SB.posUltimoBloqueAI: %u -> leer_bit(%u) = %d\n\n", SB.posUltimoBloqueAI, SB.posUltimoBloqueAI, leer_bit(SB.posUltimoBloqueAI));
    printf("SB.posPrimerBloqueDatos: %u -> leer_bit(%u) = %d\n\n", SB.posPrimerBloqueDatos, SB.posPrimerBloqueDatos, leer_bit(SB.posPrimerBloqueDatos));
    printf("SB.posUltimoBloqueDatos: %u -> leer_bit(%u) = %d\n\n", SB.posUltimoBloqueDatos, SB.posUltimoBloqueDatos, leer_bit(SB.posUltimoBloqueDatos));

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
#endif

#if DEBUG
    struct inodo inodo;
    int inodoReservado = reservar_inodo('f',6);
    bread(posSB, &SB);
    leer_inodo(inodoReservado, &inodo);

    printf("\nINODO %d - TRADUCCION DE LOS BLOQUES LOGICOS 8, 204, 30.004, 400.004 y 468.750\n",inodoReservado);
    traducir_bloque_inodo(inodoReservado,8,1);
    bread(posSB, &SB);
    traducir_bloque_inodo(inodoReservado,204,1);
    bread(posSB, &SB);
    traducir_bloque_inodo(inodoReservado,30004,1);
    bread(posSB, &SB);
    traducir_bloque_inodo(inodoReservado,400004,1);\
    bread(posSB, &SB);
    traducir_bloque_inodo(inodoReservado,468750,1);
    bread(posSB, &SB);

    printf("\nDATOS DEL INODO RESERVADO: %d\n",inodoReservado);
    struct tm *ts;
    char atime[80];
    char mtime[80];
    char ctime[80];
    ;
    leer_inodo(inodoReservado, &inodo); //Leemos el Inodo reservado
    ts = localtime(&inodo.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
    printf("tipo: %c\n", inodo.tipo);
    printf("permisos: %i\n", inodo.permisos);
    printf("ATIME: %s \nMTIME: %s \nCTIME: %s\n", atime, mtime, ctime);
    printf("nlinks: %i\n", inodo.nlinks);
    printf("tamaño en bytes lógicos: %i\n", inodo.tamEnBytesLog);
    printf("Número de bloques ocupados: %i\n", inodo.numBloquesOcupados);
    printf("SB.posPrimerInodoLibre = %d\n",SB.posPrimerInodoLibre);
#endif

    // Desmontamos el dispositivo virtual
    bumount();

    return 0;
}
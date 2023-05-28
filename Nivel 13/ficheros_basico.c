#include "ficheros_basico.h"


// Calcula el tamaño en bloques necesario para el mapa de bits.
int tamMB(unsigned int nbloques){
    int tamaño = (nbloques / 8) / BLOCKSIZE;
    // si hay resto, añadiremos un bloque de más
    if((nbloques / 8) % BLOCKSIZE > 0){
        tamaño++;
    }
    return tamaño;
}

// Calcula el tamaño en bloques del array de inodos.
int tamAI(unsigned int ninodos){
    int tamAI = (ninodos * INODOSIZE) / BLOCKSIZE;
    //Si hay resto, el tamaño del array de inodos aumenta
    if((ninodos * INODOSIZE) % BLOCKSIZE > 0){
        tamAI++;
    }
    return tamAI;
}

// Método que inicializa el Superbloque
int initSB(unsigned int nbloques, unsigned int ninodos) {
    struct superbloque SB;
    // Posicion del primer bloque del mapa de bits
    SB.posPrimerBloqueMB = posSB + tamSB;
    // Posición del último bloque del mapa de bits
    SB.posUltimoBloqueMB = SB.posPrimerBloqueMB + tamMB(nbloques) - 1;
    // Posición del primer bloque del array de inodos
    SB.posPrimerBloqueAI = SB.posUltimoBloqueMB + 1;
    // Posición del último bloque del array de inodos
    SB.posUltimoBloqueAI = SB.posPrimerBloqueAI + tamAI(ninodos) - 1;
    // Posición del primer bloque de datos
    SB.posPrimerBloqueDatos = SB.posUltimoBloqueAI + 1;
    // Posición del último bloque de datos
    SB.posUltimoBloqueDatos = nbloques - 1;
    // Posición del inodo raíz
    SB.posInodoRaiz = 0;
    // Posición del primer inodo libre (inicialmente igual al raíz)
    SB.posPrimerInodoLibre = 0;
    // Número de bloques libres (inicialmente igual al total)
    SB.cantBloquesLibres = nbloques;
    // Número de inodos libres (inicialmente igual al total)
    SB.cantInodosLibres = ninodos;
    // Número total de bloques
    SB.totBloques = nbloques;
    // Número total de inodos
    SB.totInodos = ninodos;
    
    // Se escribe el superbloque en la primera posición (posSB)
    bwrite(posSB, &SB);
    return EXITO;
}

// Inicializa el mapa de bits poniendo a 1 los bits que representan los metadatos.
int initMB() {
    unsigned char bufferMB[BLOCKSIZE];
    struct superbloque SB;
    // memset() inicializa con todo 1's el buffer y el bread() lee el SuperBloque
    // Si la memoria esta vacia o no puedo leer SB
    if (memset(bufferMB, 255, BLOCKSIZE) == NULL) {
        // Control de errores
        return FALLO;
    }
    if (bread(posSB, &SB) == FALLO){
        return FALLO;
    }
    // El tamaño en bloques del SuperBloque, el mapa de bits y el array de inodos
    int tam = SB.posPrimerBloqueDatos;
    // Restamos los bloques usados al indice del SuperBloque
    SB.cantBloquesLibres -= tam;
    // Comprobamos el número de bloques enteros del mapa de bits que se pueden crear
    // a partir de la variable tam
    int numFullBlocksMB = tam / 8 / BLOCKSIZE;
    int indice = SB.posPrimerBloqueMB;
    // Iteración para escribir un bloque entero de 1s por cada bloque de numFullBlocksMB
    for (int i = 0; i < numFullBlocksMB; i++) {
        bwrite(indice, bufferMB);
        // Incrementamos para escribir el siguiente bloque
        indice++;
    }
    
    // Llenamos todo el bufferMB de 0s
    if (memset(bufferMB, 0, BLOCKSIZE) == NULL) {
        // Control de errores
        return FALLO;
    }
    // El número de bloques de tam que no caben dentro de un bloque entero de MB
    int bloquesRestantes = (tam - (numFullBlocksMB * BLOCKSIZE * 8));
    // Número de bloques de tam que no caben en un byte
    int bloquesRestantesBinario = bloquesRestantes % 8;

    // Itera por cada byte de los bloquesRestantes
    for (int i = 0; i < (bloquesRestantes/8); i++) {
        // LLena el byte de 1's
        bufferMB[i] = 255;
    }
    // Llenamos de 1s el byte restante comenzando por la izquierda
    for (int i = 0; i < bloquesRestantesBinario; i++) {
        bufferMB[bloquesRestantes/8] += potencia(2,(7-i));
    }
    // Escribimos el bloque restante y reescribimos el SuperBloque
    bwrite(indice,bufferMB);
    bwrite(posSB, &SB);

    
    return EXITO;
}

// Función recursiva para el cálculo de potencias
int potencia(int base, int exp) {
    if (exp == 0) {
        // Cuando el exponente es 0, se devuelve 1 (x^0 = 1 para cualquier x)
        return 1;
    } else {
        // Se devuelve el producto entre la base y la potencia con el exponente de una unidad menor
        // x*(x^(n-1)) siendo x la base y n el exponente
        return base*potencia(base, exp-1);
    }
}

int initAI() {
    struct superbloque SB;
    // Lectura del contenido del superbloque
    if (bread(posSB, &SB) == FALLO) {
        // Control de errores
        return FALLO;
    }
    struct inodo inodos [BLOCKSIZE/INODOSIZE];
    int final = 0; // Se usa este flag para salir del bucle en caso de bloque incompleto
    int contInodos = SB.posPrimerInodoLibre + 1; // si hemos inicializado SB.posPrimerInodoLibre = 0
    for (size_t i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI && final == 0; i++) { // para cada bloque del AI
        if(bread(i,&inodos) == FALLO) {
            return FALLO;
        }
        for (size_t j = 0; j < BLOCKSIZE / INODOSIZE;  j++) { // para cada inodo del AI
            inodos[j].tipo = 'l'; // libre
            if (contInodos < SB.totInodos) { // si no hemos llegado al último inodo
                inodos[j].punterosDirectos[0] = contInodos; // enlazamos con el siguiente
                contInodos++;
            } else { // hemos llegado al último inodo
                inodos[j].punterosDirectos[0] = UINT_MAX;
                final = 1;
                break;
            }
        }
        // Se escribe el bloque de inodos en el dispositivo virtual
        if (bwrite(i, &inodos) == FALLO){
            // Control de errores
            return FALLO;
        }
    }
    return EXITO;   
}

int escribir_bit(unsigned int nbloque, unsigned int bit) {
    struct superbloque SB;
    // Leemos el superbloque del disco
    if (bread(posSB, &SB) == FALLO) {
        return FALLO;
    }
    unsigned char bufferMB[BLOCKSIZE];
    unsigned char mascara = 128; // 1000 0000

    // Calculamos en cuantos en que byte esta el bloque solicitado
    int posbyte = nbloque / 8;
    // El resto del último byte indicará en que bit del ultimo byte estara
    int posbit = nbloque % 8;
    // Calculamos en que bloque del Mapa de bits se encuentra el bloque solicitado
    int nbloqueMB = posbyte / BLOCKSIZE; 
    // sacamos la posición del disco donde se encuentra el bit
    int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;

    if (bread(nbloqueabs, bufferMB) == FALLO) {
        return FALLO;
    }

    posbyte = posbyte % BLOCKSIZE;
    
    mascara >>= posbit;
    if (bit == 1){
        bufferMB[posbyte] |= mascara;
    } else if (bit == 0) {
        bufferMB[posbyte] &= ~mascara;
    } else{
        return FALLO;
    }
    
    bwrite(nbloqueabs, bufferMB);
    return EXITO;
    
}

char leer_bit (unsigned int nbloque) {
    // Lectura del superbloque
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO) {
        return FALLO;
    }
    
    // Se obtiene la posición del bit
    unsigned char bufferMB[BLOCKSIZE];
    int posbyte = nbloque / 8;
    int posbit = nbloque % 8;
    int nbloqueMB = posbyte / BLOCKSIZE; 
    int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;
    // Se lee el contenido del bloque para guardarlo en un buffer
    if (bread(nbloqueabs, bufferMB) == FALLO) {
        return FALLO;
    }
    unsigned char mascara = 128; // 10000000
    mascara >>= posbit;          // desplazamiento de bits a la derecha
    mascara &= bufferMB[posbyte % BLOCKSIZE]; // operador AND para bits
    mascara >>= (7 - posbit);     // desplazamiento de bits a la derecha

    printf("[leer_bit(%i) → posbyte:%i, posbit:%i, nbloqueMB:%i, nbloqueabs:%i)]\n", nbloque, posbyte, posbit, nbloqueMB, nbloqueabs);

    return mascara;             // el resultado queda en la variable mascara
}

int reservar_bloque() {
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO) {
        return FALLO;
    }
    unsigned char bufferMB[BLOCKSIZE];
    int nbloqueabs = 1;
    int posbyte;
    int posbit;
    unsigned char mascara = 128;

    if (SB.cantBloquesLibres <= 0) {
        fprintf(stderr, "No quedan bloques libres");
        return FALLO;
    } else {
        unsigned char bufferAux[BLOCKSIZE];
        memset(bufferAux, 255, BLOCKSIZE);

        bread(nbloqueabs, bufferMB);
        while(memcmp(bufferMB, bufferAux, BLOCKSIZE) == 0){
            nbloqueabs++;
            bread(nbloqueabs, bufferMB);
        }
        
        posbyte = 0;
        for (; bufferMB[posbyte] == 255; posbyte++);
        
        posbit = 0;
        while(bufferMB[posbyte] & mascara){
            bufferMB[posbyte] <<=1;
            posbit++;
        }
        int nbloque = ((nbloqueabs - SB.posPrimerBloqueMB) * BLOCKSIZE + posbyte) * 8 + posbit;
        escribir_bit(nbloque, 1);
        SB.cantBloquesLibres--;
        bwrite(posSB, &SB);

        memset(bufferAux,0, BLOCKSIZE);
        bwrite(nbloque, bufferAux);
        
        return nbloque;

    }
}

int liberar_bloque(unsigned int nbloque){
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO) {
        return FALLO;
    }
    
    escribir_bit(nbloque, 0);
    SB.cantBloquesLibres++;
    
    bwrite(posSB, &SB);
        
    return nbloque;
}

int escribir_inodo(unsigned int ninodo, struct inodo *inodo) {
    // Lectura del superbloque
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO) {
        return FALLO;
    }
    
    // Se obtiene el bloque al cual pertenece el inodo
    unsigned int posbloqueinodo;
    struct inodo inodos[BLOCKSIZE/INODOSIZE];
    posbloqueinodo = SB.posPrimerBloqueAI + (ninodo / (BLOCKSIZE / INODOSIZE));
    
    // Se guarda en memoria el contenido del bloque
    if (bread(posbloqueinodo, inodos) == FALLO)
    {
        return FALLO;
    }
    
    // Se modifica el valor del inodo en el array
    inodos[ninodo % (BLOCKSIZE / INODOSIZE)] = *inodo;
    
    // Se reescribe el bloque una vez modificado el contenido
    bwrite(posbloqueinodo, inodos);

    return EXITO;

}

int leer_inodo(unsigned int ninodo, struct inodo *inodo) {
    // Lectura del superbloque
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO) {
        return FALLO;
    }

    // Se obtiene el bloque al cual pertenece el inodo
    unsigned int posbloqueinodo;
    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    posbloqueinodo = SB.posPrimerBloqueAI + (ninodo / (BLOCKSIZE / INODOSIZE));

    // Se guarda en el buffer inodos el contenido del bloque
    if (bread(posbloqueinodo, inodos) == FALLO)
    {
        return FALLO;
    }

    *inodo = inodos[ninodo % (BLOCKSIZE / INODOSIZE)];
    return EXITO;
}

int reservar_inodo(unsigned char tipo, unsigned char permisos) {
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO) {
        return FALLO;
    }

    // Si no hay inodos libres indicar error y salir.
    if(SB.cantInodosLibres == 0){
        return FALLO;
    }
    // Superbloque apunte al siguiente inodo libre de la lista
    int posInodoReservado = SB.posPrimerInodoLibre;
    // Actualizar la lista enlazada de inodos libres
    SB.posPrimerInodoLibre++;
    
    // Inicializamos todos los campos del inodo
    struct inodo aux;
    
    aux.tipo = tipo;
    aux.permisos = permisos;
    aux.nlinks = 1;
    aux.tamEnBytesLog = 0;
    aux.atime = time(NULL);
    aux.mtime = time(NULL);
    aux.ctime = time(NULL);
    aux.numBloquesOcupados = 0;

    for (int i = 0; i < 12; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            aux.punterosIndirectos[j] = 0;
        }
        aux.punterosDirectos[i] = 0;
    }
    // Escribir el inodo inicializado en la posición 
    if (escribir_inodo(posInodoReservado, &aux) == FALLO) {
        return FALLO;
    }

    // Cantidad de inodos libres
    SB.cantInodosLibres--;
    
    //reescribir el superbloque
    bwrite(posSB, &SB);
    
    // Devolvemos el inodo
    return posInodoReservado;
}

int obtener_nRangoBL (struct inodo *inodo, unsigned int nblogico, unsigned int *ptr) {
    if (nblogico < DIRECTOS) {
        *ptr = inodo->punterosDirectos[nblogico];
        return 0;
    } else if (nblogico < INDIRECTOS0) {
        *ptr = inodo->punterosIndirectos[0];
        return 1;
    } else if (nblogico < INDIRECTOS1) {
        *ptr = inodo->punterosIndirectos[1];
        return 2;
    } else if (nblogico < INDIRECTOS2) {
        *ptr = inodo->punterosIndirectos[2];
        return 3;
    } else {
        *ptr = 0;
        fprintf(stderr, "Bloque lógico fuera de rango");
        return -1;
    }
}

int traducir_bloque_inodo(unsigned int ninodo, unsigned int nblogico, unsigned char reservar) {
    struct inodo inodo;
    unsigned int ptr, ptr_ant;
    int indice, nRangoBL, nivel_punteros;
    int inodoModificado = 0;
    unsigned int buffer[NPUNTEROS];

    if (leer_inodo(ninodo, &inodo) == FALLO) {
        return FALLO;
    }
    
    ptr = 0;
    ptr_ant = 0;
    nRangoBL = obtener_nRangoBL(&inodo, nblogico, &ptr); //0:D, 1:I0, 2:I1, 3:I2
    // el nivel_punteros +alto es el que cuelga del inodo
    nivel_punteros = nRangoBL;
    // iterar para cada nivel de punteros indirectos
    while (nivel_punteros > 0) {
        //no cuelgan bloques de punteros
        if (ptr == 0) {
            // bloque inexistente -> no imprimir nada por pantalla!!!
            if (reservar == 0)
                return FALLO;
            else {
                ptr = reservar_bloque();
                inodoModificado = 1;
                inodo.numBloquesOcupados++;
                // Fecha actual
                inodo.ctime = time(NULL);
                // El bloque cuelga directamente del inodo
                if (nivel_punteros == nRangoBL) {
                    inodo.punterosIndirectos[nRangoBL-1] = ptr;
#if DEBUGN4
                    printf("[traducir_bloque_inodo()→ inodo.puntnerosIndirectos[%i] = %i (reservado BF %i para punteros_nivel%i)]\n", nRangoBL - 1, ptr, ptr, nivel_punteros);
#endif
                // El bloque no cuelga directamente del inodo
                } else {
                    buffer[indice] = ptr;
#if DEBUGN4
                    printf("[traducir_bloque_inodo()→ inodo.punteros_nivel%i[%i] = %i (reservado BF %i para punteros_nivel%i)]\n", nivel_punteros + 1, indice, ptr, ptr, nivel_punteros);
#endif                    
                    if (bwrite(ptr_ant, buffer) == FALLO) {
                        fprintf(stderr, "Error de escritura\n");
                        return FALLO;
                    }
                }
                memset(buffer,0,BLOCKSIZE);
            }
        }
        if (bread(ptr, buffer) == FALLO) {
            fprintf(stderr, "Error de lectura\n");
            return FALLO;
        }
        indice = obtener_indice(nblogico, nivel_punteros);
        ptr_ant = ptr;

        ptr = buffer[indice];
        nivel_punteros--;
    }

    if (ptr == 0) {
        if (reservar == 0) {
            return FALLO;
        } else {
            ptr = reservar_bloque();
            inodoModificado = 1;
            inodo.numBloquesOcupados++;
            inodo.ctime = time(NULL);
            if (nRangoBL == 0) {
                inodo.punterosDirectos[nblogico] = ptr;
#if DEBUGN4
                printf("[traducir_bloque_inodo()→ inodo.punterosDirectos[%i] = %i (reservado BF %i para BL %i)]\n", nblogico, ptr, ptr, nblogico);
#endif
            } else {
                buffer[indice] = ptr;
#if DEBUGN4
                printf("[traducir_bloque_inodo()→ inodo.punteros_nivel1[%i] = %i (reservado BF %i para BL %i)]\n", indice, ptr, ptr, nblogico);
#endif
                if (bwrite(ptr_ant, buffer) == FALLO) {
                    fprintf(stderr, "Error de escritura\n");
                    return FALLO;
                }
            }
        }
    }
    if (inodoModificado) {
        if (escribir_inodo(ninodo, &inodo) == FALLO) {
            return FALLO;
        }
    }
    return ptr;
}


int obtener_indice(unsigned int nblogico, int nivel_punteros) {
    if (nblogico < DIRECTOS) { // Nivel de bloques directos

        return nblogico;
        
    } else if (nblogico < INDIRECTOS0) { // Nivel bloques indirectos 0
        
        return nblogico - DIRECTOS;
        
    } else if (nblogico < INDIRECTOS1) { // Nivel bloques indirectos 1
        
        if (nivel_punteros == 2) {
            return (nblogico - INDIRECTOS0) / NPUNTEROS;
            
        } else if (nivel_punteros == 1) {
            
            return (nblogico - INDIRECTOS0)%NPUNTEROS;
        }
    } else if (nblogico < INDIRECTOS2) { // Nivel bloques indirectos 2
        
        if(nivel_punteros == 3){

            return (nblogico - INDIRECTOS1) / (NPUNTEROS * NPUNTEROS);
            
        }else if(nivel_punteros == 2){

            return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS))/NPUNTEROS;
            
        }else if(nivel_punteros == 1){
            
            return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS))%NPUNTEROS;
        }
    }
    return FALLO;    
}

int liberar_inodo(unsigned int ninodo){
    struct inodo inodo;
    // Leemos el inodo a liberar
    if(leer_inodo(ninodo, &inodo) == FALLO){
        return FALLO;
    }
    // liberar_bloques_inodo devolverá el numero de Bloques Liberados
    int numBloquesLiberados = liberar_bloques_inodo(0,&inodo);
    if (numBloquesLiberados == FALLO){
        fprintf(stderr, "Error al liberar los bloques de inodo");
        return FALLO;
    }
    // acualizaremos numBloquesOcupados
    inodo.numBloquesOcupados -= numBloquesLiberados;

    // Establecemos el inodo como libre y como vacio
    inodo.tipo = 'l';
    inodo.tamEnBytesLog = 0;

    struct superbloque SB;
    if(bread(posSB, &SB) == FALLO){
        return FALLO;
    }
    // Actualizamos el superbloque
    SB.posPrimerInodoLibre = ninodo;
    SB.cantInodosLibres++;
    if(bwrite(posSB, &SB) == FALLO){
        return FALLO;
    }
    inodo.ctime = time(NULL);

    // Sobreescribimos el inodo actualizado
    if(escribir_inodo(ninodo, &inodo) == FALLO){
        return FALLO;
    }

    // Devolvemos el nº inodo liberado
    return ninodo;
}

int liberar_bloques_inodo(unsigned int primerBL, struct inodo *inodo){
    unsigned int nivel_punteros, indice, ptr, nBL, ultimoBL;
    int nRangoBL;
    // Array de bloques de punteros
    unsigned int bloques_punteros[3][NPUNTEROS]; 
    // Para llenar de 0s y comparar
    unsigned char bufAux_punteros[BLOCKSIZE]; 
    // Punteros a bloques de punteros de cada nivel
    int ptr_nivel[3]; 
    // Indices de cada nivel
    int indices[3]; 
    // Nº de bloques liberados
    int liberados = 0; 
    // Inicializamos a 0s
    int nBWrites = 0;
    int nBreads = 0;

    if ((inodo->tamEnBytesLog) == 0) {
        return 0; //el fichero está vacío
    }

    // Obtenemos el último bloque lógico del inodo
    if (inodo->tamEnBytesLog % BLOCKSIZE == 0) {
        ultimoBL = ((inodo->tamEnBytesLog) / BLOCKSIZE) - 1;
    } else {
        ultimoBL = (inodo->tamEnBytesLog) / BLOCKSIZE;
    }
    
#if DEBUGN6
    printf("[liberar_bloques_inodo()-> primerBL: %d, ultimoBL: %d]\n", primerBL, ultimoBL);
#endif

    // Inicializado a 0s 
    memset (bufAux_punteros, 0, BLOCKSIZE);
    ptr = 0;
    
    // Hacemos un recorrido de los Bloques Logicos
    for (nBL = primerBL; nBL <= ultimoBL; nBL++) {
        // 0:D, 1:I0, 2:I1, 3:I2
        nRangoBL = obtener_nRangoBL(inodo, nBL, &ptr);

        // Control de errores
        if (nRangoBL < 0){
            return FALLO;
        }

        // El nivel_punteros + alto cuelga del inodo
        nivel_punteros = nRangoBL;

        while (ptr > 0 && nivel_punteros > 0) {
            
            indice = obtener_indice(nBL, nivel_punteros);

            if (indice == 0 || nBL == primerBL) {
                 // Solo hay que leer del dispositivo si no está ya cargado previamente en un buffer    
                if (bread(ptr, bloques_punteros[nivel_punteros - 1]) == FALLO) {
                    fprintf(stderr, "Error de lectura\n");
                    return FALLO;
                }
                nBreads++;
            }
            
            ptr_nivel[nivel_punteros - 1] = ptr;
            indices[nivel_punteros - 1] = indice;
            ptr = bloques_punteros[nivel_punteros - 1][indice];
            nivel_punteros--;
        }
        
        if (ptr > 0) {
            // Liberamos el bloque
            liberar_bloque(ptr);
            // Incrementamos el nº de liberados
            liberados++;

#if DEBUGN6
            printf("[liberar_bloques_inodo()-> liberado BF %d de datos par a BL %d]\n", ptr, nBL);
#endif

            if (nRangoBL == 0) { // Si es un puntero directo
                inodo->punterosDirectos[nBL] = 0;
            } else {
                nivel_punteros = 1;

                while (nivel_punteros <= nRangoBL) {
                    indice = indices[nivel_punteros - 1];
                    bloques_punteros[nivel_punteros - 1][indice] = 0;
                    ptr = ptr_nivel [nivel_punteros - 1];

                    if (memcmp(bloques_punteros[nivel_punteros - 1], bufAux_punteros, BLOCKSIZE) == 0) {
                        // No cuelgan más bloques ocupados, hay que liberar el bloque de punteros
                        liberar_bloque(ptr);
                        liberados++;

#if DEBUGN6
                        printf("[liberar_bloques_inodo()→ liberado BF %i de punteros_nivel%i correspondiente al BL: %i]\n", ptr, nivel_punteros, nBL);
#endif

                        if (nivel_punteros == nRangoBL) {
                            inodo->punterosIndirectos[nRangoBL - 1] = 0;
                        }

                        nivel_punteros++;
                    } else {
                        // Escribimos en el dispositivo el bloque de punteros modificado
                        if (bwrite(ptr, bloques_punteros[nivel_punteros - 1]) == FALLO) {
                            // Control de errores
                            fprintf(stderr, "Error de escritura\n");
                            return FALLO;
                        }
                        nBWrites++;

                        // Hemos de salir del bucle ya que no será necesario liberar los bloques de niveles
                        // superiores de los que cuelga
                        nivel_punteros = nRangoBL + 1;
                    }
                }
            }
        }
    }

#if DEBUGN6
    printf("[liberar_bloques_inodo()-> total bloques liberados: %d, total breads: %d, total bwrites: %d]\n", liberados, nBreads, nBWrites);
#endif

    // Devuelve los bloques liberados
    return liberados;
}
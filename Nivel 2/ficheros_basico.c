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
    if (memset(bufferMB, 1, BLOCKSIZE) == NULL && bread(posSB, &SB) == FALLO) {
        // Control de errores
        return FALLO;
    }
    // El tamaño en bloques del SuperBloque, el mapa de bits y el array de inodos
    int tam = SB.posPrimerBloqueDatos-1;
    // Restamos los bloques usados al indice del SuperBloque
    SB.cantBloquesLibres -= tam;
    // Comprobamos el número de bloques enteros del mapa de bits que se pueden crear
    // a partir de la variable tam
    int numFullBlocksMB = tam / 8 / BLOCKSIZE;
    int indice = SB.posPrimerBloqueMB;
    // Iteración para escribir un bloque entero de 1s por cada bloque de numFullBlocksMB
    for (int i = 0; i <= numFullBlocksMB; i++) {
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
    int bloquesRestantes = (tam - (numFullBlocksMB*BLOCKSIZE));
    // Número de bloques de tam que no caben en un byte
    int bloquesRestantesBinario = bloquesRestantes % 8;

    // Itera por cada byte de los bloquesRestantes
    for (int i = 0; i < (bloquesRestantes/8)-1; i++) {
        // LLena el byte de 1's
        bufferMB[i] = 255;
    }
    // Llenamos de 1s el byte restante comenzando por la izquierda
    for (int i = 0; i < bloquesRestantesBinario; i++) {
        bufferMB[bloquesRestantes/8] += potencia(2,(7-i));
    }
    // Escribimos el bloque restante y reescribimos el SuperBloque
    if(bwrite(indice,bufferMB) == FALLO && bwrite(posSB, &SB)){
        // Control de errores
        return FALLO;
    }
    
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
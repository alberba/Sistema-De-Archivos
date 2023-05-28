#include "ficheros.h"
int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes) {
    struct inodo inodo;
    int desp1, desp2, nbfisico;
    int numBytesEscritos = 0;
    int auxBytesEscritos = 0;
    unsigned int primerBL, ultimoBL;
    unsigned char buf_bloque[BLOCKSIZE];
    if (leer_inodo(ninodo, &inodo) == FALLO) {
        return FALLO;
    }
    
    if ((inodo.permisos & 2) != 2) {
        fprintf(stderr, "No hay permisos de escritura\n");
        return FALLO;
    }
    // Calculo de primer bloque lógico libre donde hay que escribir
    primerBL = offset / BLOCKSIZE;
    // Calculo de ultimo bloque lógico libre donde hay que escribir
    ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;

    // Desplazamiento dentro del bloque lógico de offset
    desp1 = offset % BLOCKSIZE;
    // Desplazamiento dentro del ultimo blouqe lógico de offset
    desp2 = (offset + nbytes-1) % BLOCKSIZE;
    
    nbfisico = traducir_bloque_inodo(ninodo, primerBL, 1);
    
    if (nbfisico == FALLO) {
        return FALLO;
    }


    if (primerBL == ultimoBL) {
        if (bread(nbfisico, buf_bloque) == FALLO) {
            fprintf(stderr, "Error al leer el bloque fisico\n");
            return FALLO;
        }
        memcpy(buf_bloque + desp1, buf_original, nbytes);
        // Escribimos el bloque modificado
        auxBytesEscritos = bwrite(nbfisico, buf_bloque);
        if(auxBytesEscritos == FALLO){
            fprintf(stderr, "Error al escribir el buffer modificado\n");
            return FALLO;
        }
        numBytesEscritos += nbytes;
    } else {
        // 1a fase: primer bloque lógico
        
        if (bread(nbfisico, buf_bloque) == FALLO) {
            fprintf(stderr, "Error al leer el primer bloque lógico\n");
            return FALLO;
        }
        memcpy(buf_bloque + desp1, buf_original, BLOCKSIZE - desp1);
        // Escribimos el bloque modificado
        auxBytesEscritos = bwrite(nbfisico, buf_bloque);
        if (auxBytesEscritos == FALLO) {
            fprintf(stderr, "Error al escribir el buffer modificado\n");
            return FALLO;
        }
        numBytesEscritos += auxBytesEscritos - desp1;

        // 2a fase: Bloques lógicos intermedios

        for (int i = primerBL + 1; i < ultimoBL; i++) {
            nbfisico = traducir_bloque_inodo(ninodo, i, 1);
            if(nbfisico == FALLO){
                fprintf(stderr, "Error al traducir el bloque de inodos\n");
                return FALLO;
            }
            auxBytesEscritos = bwrite(nbfisico, buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE);
            if (auxBytesEscritos == FALLO) {
                fprintf(stderr, "Error al escribir bloques intermedios\n");
                return FALLO;
            }
            numBytesEscritos += auxBytesEscritos;
        }

        // 3a fase: Bloque lógico final
        
        nbfisico = traducir_bloque_inodo(ninodo, ultimoBL, 1);

        if (nbfisico == FALLO) {
            fprintf(stderr, "Error al traducir el ultimo bloque inodo\n");
            return FALLO;
        }

        if (bread(nbfisico, buf_bloque) == FALLO) {
            fprintf(stderr, "Error al leer el primer bloque lógico\n");
            return FALLO;
        }

        memcpy(buf_bloque, buf_original + (nbytes - desp2 - 1), desp2 + 1);

        auxBytesEscritos = bwrite(nbfisico, buf_bloque);
        if(auxBytesEscritos == FALLO){
            fprintf(stderr, "Error al escribir el ultimo bloque lógico\n");
            return FALLO;
        }
        numBytesEscritos += desp2 + 1;
    }
    mi_waitSem();
    if (leer_inodo(ninodo, &inodo) == FALLO) {
        fprintf(stderr, "Error al leer el inodo actualizado\n");
        return FALLO;
    }
    
    if (inodo.tamEnBytesLog < (nbytes + offset)) {
        inodo.tamEnBytesLog = nbytes + offset;
        inodo.ctime = time(NULL);
    }

    inodo.mtime = time(NULL);
    
    if(escribir_inodo(ninodo, &inodo) == FALLO){
        fprintf(stderr, "Error al sobreescribir el inodo\n");
        return FALLO;
    }
    mi_signalSem();
    if (numBytesEscritos == nbytes) {
        return numBytesEscritos;
    } else {
        fprintf(stderr, "numBytesEscritos no coincide con nbytes\n");
        return FALLO;
    }

}

int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes) {
    // INICIALIZACIÓN
    struct inodo inodo;
    int leidos = 0;
    int auxLeidos = 0;
    int primerBloque, ultimoBloque, offsetInicio, offsetFinal, nbfisico;
    char unsigned buf_bloque[BLOCKSIZE];
    // Lectura del inodo para tener los datos en memoria
    mi_waitSem();
    if (leer_inodo(ninodo, &inodo) == FALLO) {
        // Control de errores
        fprintf(stderr, "Error al leer inodo\n");
        return leidos;
    }
    inodo.atime = time(NULL);
    if (escribir_inodo(ninodo, &inodo) == FALLO) {
        // Control de errores
        fprintf(stderr, "Error al escribir inodo\n");
        return leidos;
    }
    mi_signalSem();

    // Verificación de permisos (tienen que coincidir con 100, 101, 110 o 111, es decir, tiene que estar el bit más significativo a 1)
    if ((inodo.permisos & 4) != 4) { // Interesa detectar el valor 1XX, entonces basta hacer una AND con el número 4 en binario (100)
        fprintf(stderr, "Error: el inodo no tiene permisos de lectura (mi_read_f)\n");
        return leidos;
    }
    
    // La función no puede leer más allá del tamaño en bytes lógicos del inodo,
    if (offset >= inodo.tamEnBytesLog) {
        leidos = 0; // No se puede leer nada
        return leidos;
    }
    // Pretende leer más allá de EOF
    if ((offset + nbytes) >= inodo.tamEnBytesLog) {
        // Leemos sólo los bytes que podemos desde el offset hasta EOF
        nbytes = inodo.tamEnBytesLog - offset;
    }

    primerBloque = offset / BLOCKSIZE;
    ultimoBloque = (offset + (nbytes - 1)) / BLOCKSIZE;
    offsetInicio = offset % BLOCKSIZE;
    offsetFinal = (offset + (nbytes - 1)) % BLOCKSIZE;
    // Obtención del primer (y puede que único) bloque
    nbfisico = traducir_bloque_inodo(ninodo, primerBloque, 0);

    // LECTURA
    // Si cabe en un bloque físico
    if (primerBloque == ultimoBloque) {
        // Y no se ha devuelto -1
        if (nbfisico != FALLO) {
            // Se lee el bloque físico
            auxLeidos = bread(nbfisico, buf_bloque);
            // Control errores
            if (auxLeidos == FALLO) {
                fprintf(stderr, "Error de lectura de bloque físico (mi_read_f)\n");
                return FALLO;
            }
            memcpy(buf_original, buf_bloque + offsetInicio, nbytes);
        }
        // Se actualiza el nº de bytes leídos
        leidos = nbytes; 

    } else if (primerBloque < ultimoBloque) { // Si cabe en más de un bloque físico
        // Y no se ha devuelto -1
        if (nbfisico != FALLO) {
            // Se lee el bloque físico (primero)
            auxLeidos = bread(nbfisico, buf_bloque);
            // Control errores
            if (auxLeidos == EXIT_FAILURE) {
                fprintf(stderr, "Error de lectura de bloque físico (mi_read_f)\n");
                return EXIT_FAILURE;
            }
            memcpy(buf_original, buf_bloque + offsetInicio, BLOCKSIZE - offsetInicio);
        }
        // Se actualiza el número de bytes leídos
        leidos = BLOCKSIZE - offsetInicio;
        // Recorrido de bloques intermedios
        for (int i = primerBloque + 1; i < ultimoBloque;i++) {
            // Obtención del bloque i (intermedio)
            nbfisico = traducir_bloque_inodo(ninodo, i, 0);
            // Si no se ha devuelto -1
            if (nbfisico != FALLO) {
                // Se lee el bloque físico
                auxLeidos = bread(nbfisico, buf_bloque);
                // Control errores
                if (auxLeidos == EXIT_FAILURE) {
                    fprintf(stderr, "Error de lectura de bloque físico (mi_read_f)\n");
                    return EXIT_FAILURE;
                }
                memcpy(buf_original + (BLOCKSIZE - offsetInicio) + (i - primerBloque - 1) * BLOCKSIZE, buf_bloque, BLOCKSIZE);
            }
            // Se actualiza el nº de bytes leídos en cada iteración
            leidos += BLOCKSIZE;
        }

        // Obtención del último bloque
        nbfisico = traducir_bloque_inodo(ninodo, ultimoBloque, 0);
        // Si no se ha devuelto -1
        if (nbfisico != FALLO)
        {
            //Leemos el bloque fisico del disco
            auxLeidos = bread(nbfisico, buf_bloque);
            if (auxLeidos == EXIT_FAILURE)
            {
                fprintf(stderr, "Error in mi_read_f(): bread()\n");
                return EXIT_FAILURE;
            }
            //Calculamos el byte lógico del último bloque hasta donde hemos de leer
            memcpy(buf_original + (nbytes - offsetFinal - 1), buf_bloque, offsetFinal + 1);
        }
        // Se actualiza el nº de bytes leídos en cada iteración
        leidos += offsetFinal + 1;
    }
    mi_waitSem();
    // Lectura del inodo una vez actualizado
    if (leer_inodo(ninodo, &inodo) == FALLO) {
        // Control de errores
        fprintf(stderr, "Error al leer inodo (mi_read_f)\n");
        mi_signalSem();
        return leidos;
    }
    // Timestamp actualizado
    inodo.atime = time(NULL);
    mi_signalSem();
    // Se comprueba que se haya leído la cantidad correctad de bytes
    if (nbytes == leidos) {
        return leidos;
    } else {
        return FALLO;
    }
}

int mi_stat_f(unsigned int ninodo, struct STAT *p_stat) {
    struct inodo inodo;
    // Lectura del inodo para tener los datos en memoria
    if (leer_inodo(ninodo, &inodo) == FALLO) {
        // Control de errores
        fprintf(stderr, "Error al leer inodo (mi_stat_f)\n");
        return FALLO;
    }
    // Se copian los datos que nos interesan del inodo en el struct stat
    p_stat->tipo = inodo.tipo;
    p_stat->permisos = inodo.permisos;
    p_stat->nlinks = inodo.nlinks;
    p_stat->tamEnBytesLog = inodo.tamEnBytesLog;;
    p_stat->atime = inodo.atime;
    p_stat->ctime = inodo.ctime;
    p_stat->mtime = inodo.mtime;
    p_stat->numBloquesOcupados = inodo.numBloquesOcupados;

    return EXIT_SUCCESS;
}

int mi_chmod_f(unsigned int ninodo, unsigned char permisos){
    struct inodo inodo;
    mi_waitSem();
    //leemos nº de inodo pasado como argumento
    if(leer_inodo(ninodo, &inodo) == FALLO){
        //control errores
        fprintf(stderr, "Error al leer inodo (mi_chmod_f)\n");
        return FALLO;
    }
    
    //cambia los permisos de un fichero/directorio
    inodo.permisos = permisos;

    //actualizamos el ctime
    inodo.ctime = time(NULL);
    
    //escribimos el inodo con los permisos cambiados
    if(escribir_inodo(ninodo, &inodo) == FALLO){
        //control errores
        fprintf(stderr, "Error al escribir inodo (mi_chmod_f)\n");
        return FALLO;   
    }
    mi_signalSem();
    return EXIT_SUCCESS;
}

int mi_truncar_f(unsigned int ninodo, unsigned int nbytes) {
    struct inodo inodo;
    // Lectura del inodo para tener los datos en memoria
    if (leer_inodo(ninodo, &inodo) == FALLO) {
        // Control de errores
        fprintf(stderr, "Error al leer inodo (mi_truncar_f)\n");
        return FALLO;
    }
    // Comprobación de permisos
    if ((inodo.permisos & 2) != 2) {
        // Control de errores
        fprintf(stderr, "Error: el inodo no tiene los permisos adecuados (mi_truncar_f)\n");
        return FALLO;
    }
    // Comprobación de EOF
    if (nbytes > inodo.tamEnBytesLog) {
        fprintf(stderr, "Error EOF (mi_truncar_f)\n");
        return FALLO;
    }
    // Si no hay errores, se declaran las variables
    int liberados, primerBL;
    // Se obtiene el primer bloque
    if ((nbytes % BLOCKSIZE) == 0) {
        primerBL = nbytes / BLOCKSIZE;
    } else {
        primerBL = (nbytes / BLOCKSIZE) + 1;
    }
    // Se libera el bloque y se guarda el nº de bloques liberados
    liberados = liberar_bloques_inodo(primerBL, &inodo);
    // Actualización de metadatos
    inodo.mtime = time(NULL);
    inodo.ctime = time(NULL);
    inodo.tamEnBytesLog = nbytes;
    inodo.numBloquesOcupados -= liberados;
    if (escribir_inodo(ninodo, &inodo) ==  FALLO) {
        // Control de errores
        fprintf(stderr, "Error al actualizar el inodo (mi_truncar_f)\n");
    }

    return liberados;

}

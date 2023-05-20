// Autores: Santiago Rattenbach, Sergi Oliver y Albert Salom

#include <string.h>
#include "directorios.h"

static struct ultimaEntrada ultimaEntrada[2];

int extraer_camino(const char *camino, char *inicial, char *final, char *tipo) {
    // Siempre se comenzará por el carácter '/'
    if (camino[0] != '/') {
        return FALLO;
    }
    // Se verifica si queda algo por leer, se suma 1 a camino para evitar el primer '/'
    char *rest = strchr((camino+1), '/');
    if (rest) { // En caso afirmativo, se trata de un directorio (tipo d)
        strncpy(inicial, (camino+1), (strlen(camino) - strlen(rest) - 1));
        strcpy(final, rest);
        strcpy(tipo,"d");
    } else { // Si no queda nada por leer, entonces se trata de un fichero (tipo f)
        strcpy(inicial, (camino + 1));
        strcpy(tipo,"f");
        strcpy(final, "");
    }
    return EXITO;
}

int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos){
    struct entrada entrada;
    struct inodo inodo_dir;
    char inicial[sizeof(entrada.nombre)];
    char final[strlen(camino_parcial)];
    char tipo;
    int cant_entradas_inodo, num_entrada_inodo;

    memset(inicial, 0, sizeof(entrada.nombre));
    memset(final, 0, strlen(camino_parcial));
    memset(entrada.nombre, 0, sizeof(entrada.nombre));

    if (strcmp(camino_parcial, "/") == 0) {    
        struct superbloque SB;
        if(bread(posSB, &SB) == FALLO){
            return FALLO;
        }
        *p_inodo = SB.posInodoRaiz;
        *p_entrada = 0;
        return EXITO;
    }

    if (extraer_camino(camino_parcial, inicial, final, &tipo) == FALLO) {
        return ERROR_CAMINO_INCORRECTO;
    }
    
#if DEBUGN7
    fprintf(stderr, "[buscar_entrada()->inicial: %s, final: %s, reservar: %d]\n", inicial, final, reservar);
#endif

    // Buscamos la entrada cuyo nombre se encuentra en inicial
    leer_inodo(*p_inodo_dir, &inodo_dir);

    // Si no tiene permiso de lectura devuelve fallo
    if ((inodo_dir.permisos & 4) != 4) {
        return ERROR_PERMISO_LECTURA;
    }
  

    // Inicializar el buffer de lectura con 0s
    struct entrada buff_lec[BLOCKSIZE / sizeof(struct entrada)];
    memset(buff_lec, 0, (BLOCKSIZE / sizeof(struct entrada)) * sizeof(struct entrada));

    // Cantidad de entradas que contiene el inodo

    cant_entradas_inodo = inodo_dir.tamEnBytesLog / sizeof(entrada.nombre); 
    // Nº de entrada inicial
    num_entrada_inodo = 0;  

    if (cant_entradas_inodo > 0) {
        // Leemos la entrada
        if (mi_read_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) < 0) {
            // Control errores
            return ERROR_PERMISO_LECTURA;
        }

        while (num_entrada_inodo < cant_entradas_inodo && strcmp(inicial, entrada.nombre) != 0) {
            num_entrada_inodo++;
            memset(entrada.nombre, 0, sizeof(entrada.nombre));
            // Leemos la siguiente entrada
            if (mi_read_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) < 0) {
                // Control errores
                return ERROR_PERMISO_LECTURA;
            }
        }
    }

    if (strcmp(inicial, entrada.nombre) != 0 && (num_entrada_inodo == cant_entradas_inodo)) {
        // La entrada no existe
        switch(reservar) {
            case 0:  // Modo consulta. Como no existe retornamos error
               return ERROR_NO_EXISTE_ENTRADA_CONSULTA;
            case 1:  // Modo escritura 
                // Creamos la entrada en el directorio referenciado por *p_inodo_dir
                // si es fichero no permitir escritura
                if (inodo_dir.tipo == 'f') {
                    return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;
                }
                //si es directorio comprobar que tiene permiso de escritura
                if ((inodo_dir.permisos & 2) != 2) {
                    return ERROR_PERMISO_ESCRITURA;
                } else{
                    strcpy(entrada.nombre, inicial);
                    if (tipo == 'd') {
                        if (strcmp(final, "/") == 0) {
                            entrada.ninodo = reservar_inodo('d', permisos);
#if DEBUGN7
                            fprintf(stderr, "[buscar_entrada()->reservado inodo: %d tipo %c con permisos %d para '%s']\n", entrada.ninodo, tipo, permisos, entrada.nombre);
#endif                            
                        } else { // cuelgan más diretorios o ficheros
                            return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                        }
                    } else { // es un fichero
                        // reservar un inodo como fichero y asignarlo a la entrada 
                        entrada.ninodo = reservar_inodo('f', permisos);  
#if DEBUGN7
                        fprintf(stderr, "[buscar()->reservado inodo: %d tipo %c con permisos %d para '%s']\n", entrada.ninodo, tipo, permisos, entrada.nombre);
#endif
                    }
#if DEBUGN7
                fprintf(stderr, "[buscar_entrada()->creada entrada: %s, %d] \n", inicial, entrada.ninodo);
#endif
                    if (mi_write_f(*p_inodo_dir, &entrada, inodo_dir.tamEnBytesLog, sizeof(struct entrada)) < 0) {
                        if (entrada.ninodo != -1) { //entrada.inodo != -1
                           liberar_inodo(entrada.ninodo);
#if DEBUGN7
                        fprintf(stderr, "[buscar_entrada()-> liberado inodo %i, reservado a %s\n", num_entrada_inodo, inicial);
#endif
                        }
                        return FALLO; //-1
                    }
                }   
        }
    }

    if (!strcmp(final, "/") || !strcmp(final, "")){
        if ((num_entrada_inodo < cant_entradas_inodo) && (reservar ==1)) {
            // modo escritura y la entrada ya existe
            return ERROR_ENTRADA_YA_EXISTENTE;
        }
        // cortamos la recursividad
        // asignar a *p_inodo el numero de inodo del directorio o fichero creado o leido
        *p_inodo = entrada.ninodo;
        // asignar a *p_entrada el número de su entrada dentro del último directorio que lo contiene
        *p_entrada = num_entrada_inodo;  
        return EXITO; //0
    } else{
        // asignamos a *p_inodo_dir el puntero al inodo que se indica en la entrada encontrada;
        *p_inodo_dir = entrada.ninodo; 
        return buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
    }
    return EXITO; //0

}

void mostrar_error_buscar_entrada(int error) {
   // fprintf(stderr, "Error: %d\n", error);
   switch (error) {
   case -1: fprintf(stderr, "Error: Camino incorrecto.\n"); break;
   case -2: fprintf(stderr, "Error: Permiso denegado de lectura.\n"); break;
   case -3: fprintf(stderr, "Error: No existe el archivo o el directorio.\n"); break;
   case -4: fprintf(stderr, "Error: No existe algún directorio intermedio.\n"); break;
   case -5: fprintf(stderr, "Error: Permiso denegado de escritura.\n"); break;
   case -6: fprintf(stderr, "Error: El archivo ya existe.\n"); break;
   case -7: fprintf(stderr, "Error: No es un directorio.\n"); break;
   }
}

int mi_creat(const char *camino, unsigned char permisos) {
    // se inicializan las variables necesarias para buscar la entrada
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int error = 0;
    // Buscamos la entrada en el directorio padre
    if((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 1, permisos)) < EXITO){
        mostrar_error_buscar_entrada(error);
        return FALLO;
    }
    
    return EXITO;

}

int mi_dir(const char *camino, char *buffer) {
    // se inicializan las variables necesarias para buscar la entrada
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    // y las variables necesarias para leer el inodo
    int error = 0;
    int nEntradas = 0;
    struct inodo inodo;
    int offset = 0;
    // se busca la entrada
    if((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4)) < EXITO){
        mostrar_error_buscar_entrada(error);
        return FALLO;
    }
    // se lee el inodo
    if(leer_inodo(p_inodo, &inodo) == FALLO){
        fprintf(stderr, "Error: no se ha podido leer el inodo %d\n", p_inodo);
        return FALLO;
    }
    // se comprueba que se tienen permisos de lectura
    if((inodo.permisos & 4) != 4){
        fprintf(stderr, "Error: no se tienen permisos de lectura sobre el inodo %d\n", p_inodo);
        return FALLO;
    }

    struct entrada entradas[BLOCKSIZE / sizeof(struct entrada)];
    memset(&entradas, 0, sizeof(struct entrada));
    nEntradas = inodo.tamEnBytesLog / sizeof(struct entrada);

    offset += mi_read_f(p_inodo, entradas, offset, BLOCKSIZE);

    for(int i = 0; i < nEntradas; i++) {
        if(leer_inodo(entradas[i % (BLOCKSIZE / sizeof(struct entrada))].ninodo, &inodo) == FALLO){
            fprintf(stderr, "Error: no se ha podido leer el inodo %d\n", entradas[i%(BLOCKSIZE / sizeof(struct entrada))].ninodo);
            return FALLO;
        }

        // Tipo
        if(inodo.tipo == 'd'){
            strcat(buffer, "d");
        } else {
            strcat(buffer, "f");
        }
        strcat(buffer, "\t");

        // Permisos
        if (inodo.permisos & 4) strcat(buffer, "r"); else strcat(buffer, "-");
        if (inodo.permisos & 2) strcat(buffer, "w"); else strcat(buffer, "-");
        if (inodo.permisos & 1) strcat(buffer, "x"); else strcat(buffer, "-");
        strcat(buffer, "\t");
        
        // mtime
        char tmp[100];
        struct tm *tm;
        tm = localtime(&inodo.mtime);
        sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min,  tm->tm_sec);
        strcat(buffer, tmp);
        strcat(buffer, "\t");

        // Tamaño
        char tamBytes[sizeof(unsigned int)];
        sprintf(tamBytes, "%d", inodo.tamEnBytesLog);
        strcat(buffer, tamBytes);
        strcat(buffer, "\t");

        // Nombre
        strcat(buffer, entradas[i % (BLOCKSIZE / sizeof(struct entrada))].nombre);
        strcat(buffer, "\n");

        if(offset % (BLOCKSIZE / sizeof(struct entrada)) == 0){
            offset += mi_read_f(p_inodo, entradas, offset, BLOCKSIZE);
        }
    }
    return nEntradas;
}

int mi_chmod(const char *camino, unsigned char permisos) {
    // se inicializan las variables necesarias para buscar la entrada
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int error = 0;
    
    if((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, permisos)) < EXITO){
        mostrar_error_buscar_entrada(error);
        return FALLO;
    }

    if(mi_chmod_f(p_inodo, permisos) == FALLO) {
        return FALLO;
    }
    return EXITO;
}

int mi_stat(const char *camino, struct STAT *p_stat){
    unsigned int p_inodo = 0;
    unsigned int p_inodo_dir = 0;
    unsigned int p_entrada = 0;

    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, p_stat->permisos);
    if(error < EXITO){
        mostrar_error_buscar_entrada(error);
        return FALLO;
    }

    if(mi_stat_f(p_inodo, p_stat) == FALLO){
        return FALLO;
    }

    return p_inodo;
}

int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes) {
    // se inicializan las variables necesarias para buscar la entrada
    unsigned int p_inodo = 0;
    unsigned int p_inodo_dir = 0;
    unsigned int p_entrada = 0;
    // se verifica si la entrada ya se ha buscado anteriormente
    // con tal de optimizar el proceso
    if (strcmp(ultimaEntrada[0].camino, camino) == 0) {
        p_inodo = ultimaEntrada[0].p_inodo;
    } else { // si no se ha buscado anteriormente, se buscará ahora
        int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 2);
        if(error < EXITO){
            mostrar_error_buscar_entrada(error);
            return FALLO;
        }
        // se guarda la entrada por si se vuelve a buscar más tarde
        ultimaEntrada[0].p_inodo = p_inodo;
        strcpy(ultimaEntrada[0].camino, camino);
    }

    int nBytesEscritos = mi_write_f(p_inodo, buf, offset, nbytes);
    return nBytesEscritos;
}

int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes) {
    // se inicializan las variables necesarias para buscar la entrada
    unsigned int p_inodo = 0;
    unsigned int p_inodo_dir = 0;
    unsigned int p_entrada = 0;
    // se verifica si la entrada ya se ha buscado anteriormente
    // con tal de optimizar el proceso
    if (strcmp(ultimaEntrada[1].camino, camino) == 0) {
        p_inodo = ultimaEntrada[1].p_inodo;
    } else { // si no se ha buscado anteriormente, se buscará ahora
        int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4);
        if (error < EXITO) {
            mostrar_error_buscar_entrada(error);
            return FALLO;
        }
        // se guarda la entrada por si se vuelve a buscar más tarde
        ultimaEntrada[1].p_inodo = p_inodo;
        strcpy(ultimaEntrada[1].camino, camino);
    }
    int nBytesLeidos = mi_read_f(p_inodo, buf, offset, nbytes);
    return nBytesLeidos;
}

int mi_link(const char *camino1, const char *camino2) {
    // se inicializan las variables necesarias para buscar las entradas
    unsigned int p_inodo_dir1 = 0;
    unsigned int p_inodo1 = 0;
    unsigned int p_entrada1 = 0;

    unsigned int p_inodo_dir2 = 0;
    unsigned int p_inodo2 = 0;
    unsigned int p_entrada2 = 0;
    
    int error = 0;
    struct inodo inodo;
    // se busca la entrada de camino1
    if ((error = buscar_entrada(camino1, &p_inodo_dir1, &p_inodo1, &p_entrada1, 0, 4) <= FALLO)) {
        mostrar_error_buscar_entrada(error);
        return FALLO;
    }
    
    if (leer_inodo(p_inodo1, &inodo) == FALLO) {
        return FALLO;
    }

    if (inodo.tipo == 'd'){
        fprintf(stderr, "Error: el inodo %d es un directorio\n", p_inodo1);
        return FALLO;
    }
    // se busca la entrada de camino2
    if ((error = buscar_entrada(camino2, &p_inodo_dir2, &p_inodo2, &p_entrada2, 1, 6)) <= FALLO) {
        mostrar_error_buscar_entrada(error);
        return FALLO;
    }

    struct entrada entrada2;

    if (mi_read_f(p_inodo_dir2, &entrada2, p_entrada2 * sizeof(struct entrada), sizeof(struct entrada)) == FALLO) {
        return FALLO;
    }

    entrada2.ninodo = p_inodo1;
    
    if (mi_write_f(p_inodo_dir2, &entrada2, p_entrada2 * sizeof(struct entrada), sizeof(struct entrada)) == FALLO) {
        return FALLO;
    }
    
    if (liberar_inodo(p_inodo2) == FALLO) {
        return FALLO;
    }
    
    inodo.nlinks++;
    inodo.ctime = time(NULL);

    if (escribir_inodo(p_inodo1, &inodo) == FALLO) {
        return FALLO;
    }

    return EXITO;
    
}

int mi_unlink(const char *camino) {
    // se inicializan las variables necesarias para buscar la entrada
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int error = 0;
    // se busca la entrada del fichero
    if((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4) <= FALLO)){
        mostrar_error_buscar_entrada(error);
        return FALLO;
    }
    
    struct inodo inodo;
    if (leer_inodo(p_inodo, &inodo) == FALLO) {
        return FALLO;
    }
    // si se trata de un directorio, ha de estar vacío
    if ((inodo.tipo == 'd') && (inodo.tamEnBytesLog > 0)) {
        fprintf(stderr, "Se trata de un directorio no vacío\n");
        return FALLO;
    }

    struct inodo inodo_dir;
    if (leer_inodo(p_inodo_dir, &inodo_dir) == FALLO) {
        return FALLO;
    }
    
    // se obtiene el numero de entradas
    int numEntradas = inodo_dir.tamEnBytesLog / sizeof(struct entrada);
    
    // Si hay entrdas
    if(p_entrada != numEntradas - 1) {
        
        struct entrada entrada;
        // Se lee la ultima entrada y se coloca en la que queremos eliminar
        if (mi_read_f(p_inodo_dir, &entrada, sizeof(struct entrada) * (numEntradas - 1), sizeof(struct entrada)) < 0){
            return FALLO;
        }
        
        if (mi_write_f(p_inodo_dir, &entrada, sizeof(struct entrada) * (p_entrada), sizeof(struct entrada)) < 0) {
            return FALLO;
        }
    }

    // Se elimina la entrada
    if (mi_truncar_f(p_inodo_dir, sizeof(struct entrada)*(numEntradas - 1)) == FALLO) {
        fprintf(stderr, "Error al truncar el fichero\n");
        return FALLO;
    }

    // y se decrementa el número de enlaces
    inodo.nlinks--;

    if (!inodo.nlinks) { // si no quedan enlaces se libera el inodo
        if (liberar_inodo(p_inodo) == FALLO) {
            return FALLO;
        }
    } else { // si quedan enlaces se actualiza el inodo
        inodo.ctime = time(NULL);
        if (escribir_inodo(p_inodo, &inodo) == FALLO) {
            return FALLO;
        }
    }

    return EXIT_SUCCESS;
}


#include <string.h>
#include "directorios.h"

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
        if (mi_read_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada) < 0)) {
            // Control errores
            return ERROR_PERMISO_LECTURA;
        }

        while (num_entrada_inodo < cant_entradas_inodo && strcmp(inicial, entrada.nombre) != 0) {
            num_entrada_inodo++;
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
                        } else { // cuelgan más diretorios o ficheros
                        }
                    } else { // es un fichero
                        // reservar un inodo como fichero y asignarlo a la entrada 
                        entrada.ninodo = reservar_inodo('f', permisos);  
                    }
                    if (mi_write_f(*p_inodo_dir, &entrada, inodo_dir.tamEnBytesLog, sizeof(struct entrada)) < 0) {
                        if (entrada.ninodo != -1) { //entrada.inodo != -1
                           liberar_inodo(entrada.ninodo);
                        }
                        return FALLO; //-1
                    }
                }   
        }
    }

    if (!strcmp(final, "/") || !strcmp(final, "")){
        if ((num_entrada_inodo < cant_entradas_inodo) && (reservar ==1)) {
            //modo escritura y la entrada ya existe
            return ERROR_ENTRADA_YA_EXISTENTE;
        }
        // cortamos la recursividad
        // asignar a *p_inodo el numero de inodo del directorio o fichero creado o leido
        *p_inodo = num_entrada_inodo;
        // asignar a *p_entrada el número de su entrada dentro del último directorio que lo contiene
        *p_entrada = entrada.ninodo;  
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

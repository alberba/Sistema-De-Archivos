// Autores: Santiago Rattenbach, Sergi Oliver y Albert Salom

#include "verificacion.h"

int main (int argc, char **argv) {

    if(argc != 3){
        fprintf(stderr, "Error de sintaxis. Sintaxis correcta: ./verificacion <nombre_dispositivo> <directorio_simulación>\n");
        return FALLO;
    }

    if(bmount(argv[1]) == FALLO){
        fprintf(stderr, "Error de montaje\n");
        return FALLO;
    }

    struct STAT p_stat;
    mi_stat(argv[2], &p_stat);

#if DEBUGN13
    fprintf(stderr, "Directorio de simulación: %s\n", argv[2]);
#endif

    int numEntradas = p_stat.tamEnBytesLog / sizeof(struct entrada);

    // Si numentradas != NUMPROCESOS  entonces ERROR
    if(numEntradas != NUMPROCESOS){
        fprintf(stderr, "Error: el número de entradas no coincide con el número de procesos\n");
        bumount();
        return FALLO;
    }

#if DEBUGN13
    fprintf(stderr, "numentradas: %i, NUMPROCESOS: %i\n", numEntradas, NUMPROCESOS);
#endif

    char rutaInformeTxt[33];
    strcpy(rutaInformeTxt, argv[2]);
    strcat(rutaInformeTxt, "informe.txt");
    
    if (mi_creat(rutaInformeTxt, 7) == FALLO) {
        fprintf(stderr, "Error al crear el fichero informe.txt\n");
        return FALLO;
    }
    
    struct entrada entradas[numEntradas];
    memset(entradas, 0, sizeof(entradas));

    if (mi_read(argv[2], &entradas, 0, sizeof(entradas)) == FALLO) {
        fprintf(stderr, "Error de lectura de directorio\n");
        return FALLO;
    }

    int nbytes_info = 0;
    struct entrada entrada_aux;

    for(int i = 0 ; i < NUMPROCESOS ; i++){
        
        struct INFORMACION info;
        
        memset(&entrada_aux, 0, sizeof(struct entrada));
        // Creemos que son equivalentes | "leemos" la i-esima entrada del directorio
        // memcpy(&entrada_aux, i * sizeof(struct entrada) + buff_entrada, sizeof(struct entrada));
        memcpy(&entrada_aux, &entradas[i], sizeof(struct entrada));

        info.pid = atoi(strchr(entrada_aux.nombre, '_') + 1);
        char dir_prueba[64];

        // Crear la direccion de prueba.dat
        sprintf(dir_prueba, "%s%s", argv[2], entrada_aux.nombre);
        strcat(dir_prueba, "/prueba.dat");
        
        
        // Se podria hacer con buscar entrada, a partir del inodo que se encuentra en entrada_aux.ninodo
        // de esa manera, funcionará aunque nos cambien el nombre "prueba.dat". Si lo de arriba funciona, es mas eficiente.
        // buscar_entrada(entrada_aux.ninodo) le hace falta mas cosicas pero ahora no las tenemos
        
        int cant_registros_buffer_escrituras = 256; 
        struct REGISTRO buffer_escrituras [cant_registros_buffer_escrituras];
        memset(buffer_escrituras, 0, sizeof(buffer_escrituras)); 

        int nReg = 0; // contador de registros
        int contadorEscriturasValidadas = 0;
        
        while(mi_read(dir_prueba, buffer_escrituras, nReg * sizeof(struct REGISTRO), sizeof(buffer_escrituras)) > 0 ){
            // fprintf(stderr, "iteracion nueva del while\n");
            for(int nRegistro = 0; nRegistro < cant_registros_buffer_escrituras; nRegistro++){
                
                if(info.pid == buffer_escrituras[nRegistro].pid){ 
                    if(!contadorEscriturasValidadas){ 
                        info.PrimeraEscritura = buffer_escrituras[nRegistro];
                        info.UltimaEscritura = buffer_escrituras[nRegistro];
                        info.MayorPosicion = buffer_escrituras[nRegistro];
                        info.MenorPosicion = buffer_escrituras[nRegistro];                                       
                    }else{
                        //Actualizamos los datos de las fechas la primera y la última escritura si se necesita
                        if ((difftime(buffer_escrituras[nRegistro].fecha, info.PrimeraEscritura.fecha)) <= 0 &&
                            buffer_escrituras[nRegistro].nEscritura < info.PrimeraEscritura.nEscritura)
                        {
                            info.PrimeraEscritura = buffer_escrituras[nRegistro];
                        }
                        if ((difftime(buffer_escrituras[nRegistro].fecha, info.UltimaEscritura.fecha)) >= 0 &&
                            buffer_escrituras[nRegistro].nEscritura > info.UltimaEscritura.nEscritura)
                        {
                            info.UltimaEscritura = buffer_escrituras[nRegistro];
                        }
                        if (buffer_escrituras[nRegistro].nRegistro < info.MenorPosicion.nRegistro)
                        {
                            info.MenorPosicion = buffer_escrituras[nRegistro];
                        }
                        if (buffer_escrituras[nRegistro].nRegistro > info.MayorPosicion.nRegistro)
                        {
                            info.MayorPosicion = buffer_escrituras[nRegistro];
                        }
                       
                    }   
                    
                    contadorEscriturasValidadas++;
                    info.nEscrituras = contadorEscriturasValidadas;
                    
                    
                }
            }

            nReg += cant_registros_buffer_escrituras; // Hemos leido cant_registros_buffer_escrituras
            memset(buffer_escrituras, 0, sizeof(buffer_escrituras));     
            
        }

#if DEBUGN13
        fprintf(stderr, "[%i) %i escrituras validadas en %s]\n", i, contadorEscriturasValidadas, dir_prueba);
#endif

        // Añadimos la informacion del struct info en el fichero
        
        char buffer[BLOCKSIZE];
        memset(buffer, 0, BLOCKSIZE);
        sprintf(buffer, "PID: %d\nNúmero de escrituras: \t%d\nPrimera Escritura\t%d\t%d\t%s\nÚltima Escritura\t%d\t%d\t%s\nMenor Posición\t\t%d\t%d\t%s\nMayor Posición\t\t%d\t%d\t%s\n", 
                info.pid, info.nEscrituras, info.PrimeraEscritura.nEscritura, info.PrimeraEscritura.nRegistro, asctime(localtime(&info.PrimeraEscritura.fecha)),
                info.UltimaEscritura.nEscritura, info.UltimaEscritura.nRegistro, asctime(localtime(&info.UltimaEscritura.fecha)), info.MenorPosicion.nEscritura,
                info.MenorPosicion.nRegistro, asctime(localtime(&info.MenorPosicion.fecha)), info.MayorPosicion.nEscritura, info.MayorPosicion.nRegistro, 
                asctime(localtime(&info.MayorPosicion.fecha)));
        
        // Añadir la información del struct info al fichero informe.txt
        int bytesLeidos = mi_write(rutaInformeTxt, buffer, nbytes_info, strlen(buffer)*sizeof(char));
        if(bytesLeidos == FALLO){
            fprintf(stderr, "Error de escritura en el fichero informe.txt\n");
            return FALLO;
        }
        nbytes_info += bytesLeidos;
    }
    bumount();
}
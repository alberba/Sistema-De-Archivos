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
        return FALLO;
    }

#if DEBUGN13
    fprintf(stderr, "numentradas: %i, NUMPROCESOS: %i\n", numEntradas, NUMPROCESOS);
#endif

    char rutaInformeTxt[33];
    strcpy(rutaInformeTxt, argv[2]);
    strcat(rutaInformeTxt, "informe.txt");
    
    if (mi_creat(rutaInformeTxt, 6) == FALLO) {
        fprintf(stderr, "Error al crear el fichero informe.txt\n");
        return FALLO;
    }
    
    struct entrada entradas[numEntradas];

    if (mi_read(argv[2], entradas, 0, sizeof(struct entrada) * numEntradas) == FALLO) {
        fprintf(stderr, "Error de lectura de directorio\n");
    }

    int nbytes_info = 0;
    for (int nEntrada = 0; nEntrada < numEntradas; nEntrada++) {
        // Se lee la entrada y se extrae el pid
        // y se guarda en un registro info
        struct INFORMACION info;
        memset(&info, 0, sizeof(info));
        info.pid = atoi(strchr(entradas[nEntrada].nombre, '_') + 1);

        char ficheroPrueba[46];
        strcpy(ficheroPrueba, argv[2]);
        strcat(ficheroPrueba, entradas[nEntrada].nombre);
        strcat(ficheroPrueba, "/prueba.dat");

        int cant_registros_buffer_escrituras = 256; 
        struct REGISTRO buffer_escrituras [cant_registros_buffer_escrituras];
        memset(buffer_escrituras, 0, sizeof(buffer_escrituras));
        int offset = 0;

        while (mi_read(ficheroPrueba, buffer_escrituras, offset, sizeof(buffer_escrituras)) > 0) {
            int nRegistro = 0;
            // Se verifica que la estructura sea válida
            if (buffer_escrituras[nRegistro].pid == info.pid) {
                // Se mira si es la primera estructura validada
                if (info.nEscrituras == 0) {
                    // y se inicializan los registros con sus datos
                    info.MenorPosicion = buffer_escrituras[nRegistro];
                    info.MayorPosicion = buffer_escrituras[nRegistro];
                    info.PrimeraEscritura = buffer_escrituras[nRegistro];
                    info.UltimaEscritura = buffer_escrituras[nRegistro];
                } else {

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

                    /*// Comparar nº de escritura (para obtener primera y última, así como la mayor posición y la menor) y actualizarlas si es preciso
                    // Primera estructura
                    if (buffer_escrituras[nRegistro].nEscritura < info.PrimeraEscritura.nEscritura) {
                        info.PrimeraEscritura = buffer_escrituras[nRegistro];
                    }
                    // Última estructura
                    if (buffer_escrituras[nRegistro].nEscritura > info.UltimaEscritura.nEscritura) {
                        info.UltimaEscritura = buffer_escrituras[nRegistro];
                    }
                    // Mayor posición
                    if (buffer_escrituras[nRegistro].nRegistro > info.MayorPosicion.nRegistro) {
                        info.MayorPosicion = buffer_escrituras[nRegistro];
                    }
                    // Menor posición
                    else if (buffer_escrituras[nRegistro].nRegistro < info.MenorPosicion.nRegistro) {
                        info.MenorPosicion = buffer_escrituras[nRegistro];
                    }*/
                }
                info.nEscrituras++;
            }
            nRegistro++;

            // Reseteamos el buffer de escrituras
            memset(buffer_escrituras, 0, sizeof(buffer_escrituras));
            offset += sizeof(buffer_escrituras);
        }

#if DEBUGN13
        fprintf(stderr, "[%i) %i escrituras validadas en %s]\n", nEntrada + 1, info.nEscrituras, ficheroPrueba);
#endif

        //Añadimos la informacion del struct info en el fichero
        
        char buffer[BLOCKSIZE];
        memset(buffer, 0, BLOCKSIZE);

        sprintf(buffer, "PID: %d\nNúmero de escrituras: %d", info.pid, info.nEscrituras);
        sprintf(buffer,"Primera Escritura\t%d\t%d\t%s", info.PrimeraEscritura.nEscritura, info.PrimeraEscritura.nRegistro, asctime(localtime(&info.PrimeraEscritura.fecha)));
        sprintf(buffer, "Última Escritura\t%d\t%d\t%s", info.UltimaEscritura.nEscritura, info.UltimaEscritura.nRegistro, asctime(localtime(&info.UltimaEscritura.fecha)));
        sprintf(buffer, "Menor Posición\t%d\t%d\t%s", info.MenorPosicion.nEscritura, info.MenorPosicion.nRegistro, asctime(localtime(&info.MenorPosicion.fecha)));
        sprintf(buffer, "Mayor Posición\t%d\t%d\t%s", info.MayorPosicion.nEscritura, info.MayorPosicion.nRegistro, asctime(localtime(&info.MayorPosicion.fecha)));
        // Añadir la información del struct info al fichero informe.txt
        if(nbytes_info += mi_write(rutaInformeTxt, buffer, nbytes_info, BLOCKSIZE) == FALLO){
            fprintf(stderr, "Error de escritura en el fichero informe.txt\n");
            return FALLO;
        }
    }
    bumount();
}
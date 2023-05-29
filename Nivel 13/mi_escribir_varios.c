// Autores: Santiago Rattenbach, Sergi Oliver y Albert Salom

#include "directorios.h"


int main(int argc, char **argv){

  //Comprobamos sintaxis
  if (argc!=5) {
    fprintf(stderr, RED "Sintaxis: mi_escribir <nombre_dispositivo> </ruta_fichero> <texto> <offset>\n" RESET);
    exit(-1);
   }

  //struct STAT stat;

  //montamos el dispositivo
  if(bmount(argv[1])<0) return -1;
  //obtenemos el texto y su longitud
  char *buffer_texto = argv[3];
  int longitud=strlen(buffer_texto);

  //obtenemos la ruta y comprobamos que no se refiera a un directorio
  if (argv[2][strlen(argv[2])-1]=='/') {
    fprintf(stderr, RED "Error: la ruta se corresponde a un directorio.\n" RESET);
    exit(-1);
  }
  char *camino = argv[2];
  //obtenemos el offset
  unsigned int offset=atoi(argv[4]);
  
  int escritos=0;
  int varios = 10;
  fprintf(stderr, RED "longitud texto: %d\n" RESET, longitud);
  for (int i=0; i<varios; i++) {
    // escribimos varias veces el texto desplazado 1 bloque
    escritos += mi_write(camino,buffer_texto,offset+BLOCKSIZE*i,longitud); 
  }
  fprintf(stderr, RED "Bytes escritos: %d\n" RESET, escritos);

  bumount();
}

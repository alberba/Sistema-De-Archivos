Miembros del grupo:
    Sergi Oliver
    Santiago Rattenbach
    Albert Salom

Sintaxis Espacífica: 
    - mi_mkfs.c: mi_mkfs
    - escribir.c: escribir <nombre_dispositivo> <"$(cat fichero)"> <diferentes_inodos>
    - leer.c: leer <nombre_dispositivo> <numero_inodo>
    - permitir.c: permitir <nombre_dispositivo>
    - leer_sf.c: leer_sf <nombre_dispositivo>
    - mi_mkdir.c: ./mi_mkdir
    - mi_cat.c: ./mi_cat </ruta_fichero>
    - mi_chmod.c: ./mi_mkdir
    - mi_escribir_varios.c: mi_escribir <nombre_dispositivo> </ruta_fichero>
    - mi_link.c: ./mi_link disco /ruta_fichero_original /ruta_enlace
    - mi_ls.c: ./mi_ls </ruta_directorio>
    - mi_rm.c: ./mi_rm </ruta_fichero>
    - mi_stat.c: ./mi_mkdir
    - mi_touch.c: /mi_touch
    - simulacuon.c: ./simulacion
    - verificacion.c: ./verificacion <nombre_dispositivo> <directorio_simulación>
    - truncar.c: ./truncar <nombre_dispositivo>

Mejoras realizadas:
    - Caché de directorios: Hemos implementado el CACHE de directorios utilizando una tabla de rutas FIFO.
    - Información de las entradas en el mi_ls, junto con colores para hacerlo más legible.
    - Implementación del mapeado del disco utilizando el mmap, mejorando asi tiempos de ejecución.
    
Consideraciones:    
    -No hemos considerado necesario implementar los cambios del rmdir.c y el mi_touch.c
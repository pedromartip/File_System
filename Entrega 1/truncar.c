//Carlos Ecker, Pedro Marti y Alejandro Medina 
#include "ficheros.h"
//Sintaxis: truncar <nombre_dispositivo> <ninodo> <nbytes>
int main(int argc, char **argv){
    char descriptor[1024];
    struct STAT st;
    if (argc != 4){
        fprintf(stderr, "Sintaxis: truncar <nombre_dispositivo> <ninodo> <nbytes>\n");
        return -1;
    }
    strcpy(descriptor, argv[1]);
    if (bmount(descriptor) == -1){
        fprintf(stderr, "Error al montar el dispositivo\n");
        return -1;
    }
       
    int ninodo = atoi(argv[2]);
    int nbytes = atoi(argv[3]);
    mi_truncar_f(ninodo, nbytes);
    if (mi_stat_f(ninodo, &st) == -1){
        fprintf(stderr, "Error stat\n");
        return -1;
    }
    fprintf(stderr,"DATOS INODO %d\n", ninodo);
    fprintf(stderr,"Tipo %c\n", st.tipo);
    fprintf(stderr,"Permisos %c\n", st.permisos);
    fprintf(stderr,"tamEnBytesLog %d\n", st.tamEnBytesLog);
    fprintf(stderr,"nlinks %d\n", st.nlinks);
    fprintf(stderr,"numBloquesOcupados %d\n", st.numBloquesOcupados);

    if (bumount() == -1){
        fprintf(stderr, "Error al montar el dispositivo\n");
        return -1;
    }
    return 0;
}
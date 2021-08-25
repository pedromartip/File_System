//Carlos Ecker, Pedro Marti y Alejandro Medina 
#include "ficheros.h"

int main(int argc, char **argv){

    
    int TamBuf = 1500, ninodo, offset, leidos, total_leidos;
    char descriptor[1024], string[128], buf[TamBuf];
    struct STAT st;
    if(argc != 3){
        fprintf(stderr,"Sintaxis: Leer,Nombre dsipositivo,ninodo\n");
        return -1;
    }
    
    strcpy(descriptor, argv[1]);
    if (bmount(descriptor) == -1){
        fprintf(stderr, "Error al montar el dispositivo");
        return -1;
    }

    ninodo = atoi(argv[2]);
    offset = 0;
    total_leidos=0;
    memset(buf, 0, TamBuf);

    while ((leidos = mi_read_f(ninodo, buf, offset, TamBuf)) > 0){
        write(1, buf, leidos);
        memset(buf, 0, TamBuf);
        total_leidos = total_leidos+leidos;
        offset = offset + TamBuf;
    }
    
    sprintf(string, "total_leidos %d\n", total_leidos);
    write(2, string, strlen(string));
    
    if (mi_stat_f(ninodo, &st) == -1){
        fprintf(stderr, "Error en el Stat");
        return -1;
    }

    sprintf(string, "tamEnBytesLog %d\n", st.tamEnBytesLog); 
    write(2, string, strlen(string));
    if (bumount() == -1){
        fprintf(stderr, "Error al desmontar");
        return -1;
    }


    return 0;
}
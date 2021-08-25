//Carlos Ecker, Pedro Marti y Alejandro Medina 
#include "ficheros.h"


//permitir <nombre_dispositivo> <ninodo> <permisos>
int main(int argc, char **argv){
    char descriptor[1024];
    int x;

     if (argc != 4){
        fprintf(stderr, "Sintaxis: escribir <nombre_dispositivo> <ninodo> <permisos>\n");
        return -1;
    }
    strcpy(descriptor, argv[1]);    
    int ninodo = atoi(argv[2]);
    
    if (bmount(descriptor) == -1){
        fprintf(stderr, "Error al montar el dispositivo\n");
        return -1;
    }
    x = atoi(argv[3]);
    mi_chmod_f(ninodo, x);
 
    if (bumount() == -1){
        fprintf(stderr, "Error al desmontar\n");
        return -1;
    }
        
    return 0;
}
//Carlos Ecker, Pedro Marti y Alejandro Medina 
#include "directorios.h"

int main(int argc, char **argv) {
    char descriptor[1024];
    int permisos;
    if(argc != 4){
        printf("Sintaxis: ./mi_touch <disco> <permisos> </ruta>\n");
        return -1;
    }
        strcpy(descriptor, argv[1]);
    if(bmount(descriptor)==-1){
        fprintf(stderr,"Error al montar el dispositivo");
        return -1;
    }
	permisos = atoi(argv[2]);
    if((0>permisos)||(permisos>7)){
        fprintf(stderr,"Error en los permisos");
        return -1;
    }
    if((argv[3][strlen(argv[3])-1])!='/'){
        if(mi_creat(argv[3],permisos)==-1){
            
            return -1;
        }
    }else{
        fprintf(stderr,"ruta no valida\n");
            return -1;
    }
        		
	if(bumount()==-1){
        fprintf(stderr,"Error al desmontar el dispositivo");
        return -1;
    }
	return 0;
}
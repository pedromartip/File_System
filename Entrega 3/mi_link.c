//Carlos Ecker, Pedro Marti y Alejandro Medina 
#include "directorios.h"

int main(int argc, char **argv) {
    char descriptor[1024];
    
    if(argc != 4){
        printf("./mi_link disco /ruta_fichero_original /ruta_enlace\n");
        return -1;
    }
    
    strcpy(descriptor, argv[1]);
    if(bmount(descriptor)==-1){
        fprintf(stderr,"Error al montar el dispositivo");
        return -1;
    }
	
    mi_link(argv[2], argv[3]);
		
	
	if(bumount()==-1){
        fprintf(stderr,"Error al desmontar el dispositivo");
        return -1;
    }
	return 0;
}
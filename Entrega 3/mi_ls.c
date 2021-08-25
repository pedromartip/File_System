//Carlos Ecker, Pedro Marti y Alejandro Medina 
#include "directorios.h"

int main(int argc, char **argv){
	
	char descriptor[1024];
	char buffer[50000];
	char tipo = '\0';
	
	if(argc != 3){
		printf("Sintaxis: ./mi_ls <disco> </ruta_directorio>n");
		return -1;
	}

	

	if((argv[2][strlen(argv[2])-1])=='/'){
	 	tipo ='d';
	}
	else{
	 	tipo ='f';
	}
	
    strcpy(descriptor, argv[1]);
	if(bmount(descriptor) == -1){
        fprintf(stderr,"Error al montar el dispositivo\n");
        return -1;
    }
    


	mi_dir(argv[2],buffer,&tipo);
	

	if((tipo =='d') && ((argv[2][strlen(argv[2])-1]) != '/')){
		fprintf(stderr,"error permisos\n");
		return -1;
	} 

	fprintf(stderr,"Tipo\tModo      mtime\t\t\tTamaño\tNombre\n");
	fprintf(stderr,"-------------------------------------------------------\n%s",buffer);

	if(bumount() == -1){
        fprintf(stderr,"Error al desmontar el dispositivo\n");
        return -1;
    }

	return 0;
}


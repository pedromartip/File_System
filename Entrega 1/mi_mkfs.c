//Carlos Ecker, Pedro Marti y Alejandro Medina 
#include "ficheros.h"

unsigned char buf[BLOCKSIZE];
int TamBuf = sizeof(buf);



int main(int argc, char **argsv){
    // int byte_es=0;
    if(argc!=3){
        fprintf(stderr,"Argumentos no válidos");
    }else{
        if(bmount(argsv[1])==-1){
            return -1;
        };
        char buff[BLOCKSIZE];
        memset(buff,0,sizeof(buff));
        int x = atoi(argsv[2]); //cast de string a int .Hay que hacer tratamiento de errores
        for (int i = 0; i < x; i++){//generamos los datos de los bloques
            bwrite(i,buff);
            memset(buff,0,sizeof(buff));
        }
        
        initSB(x,x/4 );
        initMB();
        initAI();
        reservar_inodo('d',7);
          
        // traducir_bloque_inodo(1,8,1);
          
        // traducir_bloque_inodo(2,204,1);
        // traducir_bloque_inodo(3,30004,1);
        // traducir_bloque_inodo(4,400004,1);
        // traducir_bloque_inodo(4,468750,1);
        if(bumount()==-1){
            return -1;
        };
        
        
    }
}
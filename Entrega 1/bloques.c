//Carlos Ecker, Pedro Marti y Alejandro Medina 
#include "bloques.h"
#include "semaforo_mutex_posix.h"

//static sem_t *mutex;
static int descriptor = 0;
static unsigned int inside_sc = 0;


int bmount(const char *camino){
    
    if (descriptor > 0){
       close(descriptor);
    }

    mode_t mode = 0666;
    int oflags = O_RDWR | O_CREAT;
    if((descriptor = open(camino, oflags, mode)) == -1){
        fprintf(stderr, "(bloques.c) --> bmount/open error\n");
    }
    /*if (!mutex) { // el semáforo es único en el sistema y sólo se ha de inicializar 1 vez (padre)
        mutex = initSem(); 
        if (mutex == SEM_FAILED) {
            return -1;
        }
    }*/

    return descriptor;
}
int bumount(){
    descriptor =  close(descriptor);
    if(descriptor == -1){
        fprintf(stderr, "(bloques.c) --> bumount/close error\n");
    }

    //deleteSem(); //Eliminaremos el semáforo
    return 0;
}

int bwrite(unsigned int nbloque, const void *buf){
    
    if(lseek(descriptor,nbloque*BLOCKSIZE,SEEK_SET)==-1){
        fprintf(stderr,"Error seek");
        return -1;
    }
    
    return write(descriptor,buf,BLOCKSIZE);
}
int bread (unsigned int nbloque,void *buf){
    lseek(descriptor,nbloque*BLOCKSIZE,SEEK_SET);
    int bytes_leidos = read(descriptor,buf,BLOCKSIZE);
    return bytes_leidos ;
}
/*void mi_waitSem() {
    if (!inside_sc) { //inside_sc==0
       waitSem(mutex);
    }
    inside_sc++;
}
 
void mi_signalSem(){
   inside_sc--;
   if (!inside_sc){
       signalSem(mutex);
   }
}*/
//Carlos Ecker, Pedro Marti y Alejandro Medina 
#include "ficheros_basico.h"

struct inodo inodos [BLOCKSIZE/INODOSIZE];
#define DEBUGTraducir 0 //Debug traducir_bloque_inodo
#define DEBUGLiberar 0 //Debug liberar_bloque_inodo

int tamMB(unsigned int nbloques){
    int tamMB = (nbloques/8);
    if (tamMB%BLOCKSIZE==0){
        return tamMB/BLOCKSIZE;
    }else{
        return (tamMB/BLOCKSIZE) + 1;
    }   
} 


int tamAI(unsigned int ninodos){
    
    int  tamAI = (ninodos * INODOSIZE);
    if (tamAI%BLOCKSIZE==0){
        return (tamAI/BLOCKSIZE); 
    }else{
        return (tamAI/BLOCKSIZE)+1;
    }
}

int initSB(unsigned int nbloques, unsigned int ninodos){
    SB.posPrimerBLoqueMB = posSB+tamSB;
    SB.posUltimoBLoqueMB = SB.posPrimerBLoqueMB+tamMB(nbloques)-1;
    SB.posPrimerBLoqueAI = SB.posUltimoBLoqueMB+1;
    SB.posULtimoBLoqueAI = SB.posPrimerBLoqueAI+tamAI(ninodos)-1;
    SB.posPrimerBLoqueDatos = SB.posULtimoBLoqueAI+1;
    SB.posUltimoBloqueDatos = nbloques-1;
    SB.posInodoRaiz = 0;
    
    //POSICION DEL PRIMER INODO LIBRE EN EL ARRAY DE INODOS
    SB.posPrimerInodoLibre=0;
    SB.cantBloquesLibres=nbloques;
    SB.cantInodosLibres=ninodos;
    SB.totBloques=nbloques;
    SB.totInodos=ninodos;
    
    return bwrite(posSB, &SB);

}

int initMB(){
    void *bufferMB[BLOCKSIZE];
    
    memset(bufferMB,0,sizeof(bufferMB));
    for (int i = SB.posPrimerBLoqueMB; i <= SB.posUltimoBLoqueMB; i++){
        
        if (bwrite(i,bufferMB) == -1){
            return -1;
        }
        
    }
    for (int i = 0; i < (SB.posPrimerBLoqueDatos); i++){
       reservar_bloque();
    }
    
    return 0;
    
}

int initAI(){
    int x=0;
    struct inodo inodos [BLOCKSIZE/INODOSIZE];
    int continodos= SB.posPrimerInodoLibre+1;
    for (int i = SB.posPrimerBLoqueAI; (i <= SB.posULtimoBLoqueAI) && (x==0); i++){
        for (int j = 0; j< BLOCKSIZE/INODOSIZE; j++)
        {
            inodos[j].tipo='l';
            if(continodos<SB.totInodos){
                inodos[j].punterosDirectos[0]=continodos;
                continodos++;
            }else{
                inodos[j].punterosDirectos[0]=UINT_MAX;
                x=1;
                break;
            }
        }
        if(bwrite(i,inodos)==-1){
            fprintf(stderr,"(initAI) --> Error escritura");
            return -1;
        }
        memset(inodos,'0',sizeof(struct inodo)*8);
    }
    
     
    return 0;
}

 int escribir_bit(unsigned int nBloque, unsigned int bit)
 {
     unsigned char bufferMB[BLOCKSIZE];
     unsigned char mascara = 128;
     struct superbloque SB;
       unsigned int posbit;
      unsigned int posbyte;
     
      if(bread(0,&SB)==-1){
          fprintf(stderr,"(escribir_bit) --> Error lectura");
          return -1;
      }
     //calculamos la posición del byte en el MB
     posbyte = nBloque / (BLOCKSIZE/INODOSIZE);
     posbit = nBloque % (BLOCKSIZE/INODOSIZE);
     //posicion absoluta del dispositivo virtual en el que se encuentra el bloque
     int poSBlock = (posbyte / BLOCKSIZE) + SB.posPrimerBLoqueMB;
     if (bread(poSBlock, bufferMB) == -1)
     {
         printf("Error en leyendo el bloque en escribir bit\n");
         return -1;
     }
     //utilizamos mascara para desplazar bits a la derecha
     mascara >>= posbit;
     //localizamos posición de poSByte
     posbyte = posbyte % BLOCKSIZE;
     //instrucción para poner a 0 o a 1 el bit correspondiente
    if (bit==1)
    {
        bufferMB[posbyte]|=mascara;
    }else
    {
        bufferMB[posbyte]&=mascara;
    }
     //escribimos buffer del MB en dispositivo virtual
     if (bwrite(poSBlock, bufferMB) == -1)
     {
         printf("Error en escribir el bloque\n");
         return -1;
     }

    
     return 0;
 }

char leer_bit (unsigned int nbloque){
    unsigned char bufferMB[BLOCKSIZE];
    unsigned char mascara = 128;
    if(bread(0,&SB)==-1){
        fprintf(stderr,"(leer_bit) --> Error lectura");
    }   
    // int posbyte= nbloque/(BLOCKSIZE/INODOSIZE)
    int posbyte=nbloque/8;
    // int posbIt= nbloque%(BLOCKSIZE/INODOSIZE)
    int posbit=nbloque%8;
    int nbloqueMB=posbyte/BLOCKSIZE;
    int nbloqueabs = SB.posPrimerBLoqueMB+nbloqueMB;
    posbyte = posbyte%BLOCKSIZE;
    
    
    mascara>>=posbit;
    mascara&=bufferMB[posbyte];
    mascara>>=(7-posbit);
    printf("\n(escribir_bit) --> posbyte:%d,posbit:%d,nbloquesMB:%d,nbloqueabs:%d",posbyte,posbit,nbloqueMB,nbloqueabs);
    printf("\nleer_bit(%d) = %c",nbloque,mascara);
    return mascara;

}


int reservar_bloque(){
   
    unsigned int posbit;
    unsigned int posbyte;
    unsigned char bufferMB[BLOCKSIZE];
    unsigned char bufferAux[BLOCKSIZE];
    unsigned char mascara = 128;
    if(bread(posSB,&SB)==-1){
        fprintf(stderr,"(reservar_bloque) --> Error lectura");
        return -1;
    }  
      
    if(SB.cantBloquesLibres>0){
        unsigned int posbloqueMB = SB.posPrimerBLoqueMB;
        int encontrado=0;//boolean
        memset(bufferAux,255,BLOCKSIZE);
        while (encontrado==0){
            if(posbloqueMB>SB.posUltimoBLoqueMB){
                fprintf(stderr,"(reservar_bloque) --> Error tam\n");
                return -1;
            } 
            if(bread(posbloqueMB,bufferMB)==-1){
                fprintf(stderr,"(reservar_bloque) --> Error lectura");
                return -1; 
            }
 
            if(memcmp(bufferMB,bufferAux,BLOCKSIZE)!=0){
                encontrado=1;
                
            }else{
                memset(bufferAux,255,BLOCKSIZE);
                posbloqueMB++;
            }
        }
        posbyte=0;
         while(bufferMB[posbyte]>= 255){
		 	posbyte++;
		 }
        

        posbit=0;
        while(bufferMB[posbyte]&mascara){
            bufferMB[posbyte]<<=1;
            posbit++;
        }
        unsigned int nbloque=((posbloqueMB-SB.posPrimerBLoqueMB)*BLOCKSIZE+posbyte)*8+posbit;

        escribir_bit(nbloque,1);
        SB.cantBloquesLibres--;
        memset(bufferAux,0,BLOCKSIZE);
        memset(bufferMB,0,BLOCKSIZE);

        if(bwrite(posSB,&SB)==-1){
            fprintf(stderr,"(reservar_bloque) --> Error escritura");
            return -1;
        }
        return nbloque; 
    }else{
        fprintf(stderr,"(reservar_bloque) --> No hay bloques libres");
        return -1;
    }
} 

int liberar_bloque (unsigned int nbloque){
    if(bread(0,&SB)==-1){
        fprintf(stderr,"(liberar_bloque) --> Error lectura");
        return -1;
    }
    escribir_bit(nbloque,0);
    SB.cantBloquesLibres++;
    if(bwrite(0,&SB)==-1){
        fprintf(stderr,"(liberar_bloque) --> Error escritura");
        return -1;
    }
    return nbloque;
}


int escribir_inodo(unsigned int ninodo, struct inodo inodo){
    struct inodo inodos [BLOCKSIZE/INODOSIZE];

    if(bread(0,&SB)==-1){
        fprintf(stderr,"(escribir_inodo) --> Error lectura");
        return -1;
    }

    int x;
    int posbyte=ninodo/(BLOCKSIZE/INODOSIZE);
    int posbit=ninodo%(BLOCKSIZE/INODOSIZE);
    int nbloqueabs = SB.posPrimerBLoqueAI+posbyte;

    if(bread(nbloqueabs,inodos)==-1){
        fprintf(stderr,"(escribir_inodo) --> Error lectura");
        return -1;
    }

    inodos[posbit]=inodo;
    x = bwrite(nbloqueabs,inodos);

    if(x==-1){
        fprintf(stderr,"(escribir_inodo) --> Error escritura");
        return -1;
    }
    return x;
    
}

int leer_inodo(unsigned int ninodo, struct inodo *inodo){

    if(bread(0,&SB) == -1){
        fprintf(stderr,"(leer_inodo) --> Error lectura");
        return -1;
    }
    int posbyte=ninodo/(BLOCKSIZE/INODOSIZE);
    int posbit=ninodo%(BLOCKSIZE/INODOSIZE);
    int nbloqueabs = SB.posPrimerBLoqueAI+posbyte;
    if(bread(nbloqueabs,inodos)==-1){
        fprintf(stderr,"(leer_inodo) --> Error lectura");
        return -1;
    }
    *inodo=inodos[posbit];
    return 0;
}



int reservar_inodo(unsigned char tipo, unsigned char permisos){
    if(bread(0,&SB)==-1){
        fprintf(stderr,"(reservar_inodo) --> Error lectura");
        return -1;
    }
    if (SB.cantInodosLibres>0)
    {
        int posInodoReservado = SB.posPrimerInodoLibre;
        SB.posPrimerInodoLibre++;
        struct inodo inodos;
        inodos.tipo = tipo;
        inodos.permisos = permisos;
        inodos.nlinks = 1;
        inodos.tamEnBytesLog=0;
        inodos.numBloquesOcupados=0;
        inodos.atime = time(NULL);
        inodos.mtime = time(NULL);
        inodos.ctime = time(NULL);
        memset(inodos.punterosDirectos,0,sizeof(unsigned int)*12);
        memset(inodos.punterosIndirectos,0,sizeof(unsigned int)*3);
        escribir_inodo(posInodoReservado,inodos);
        SB.cantInodosLibres--;
        
        if (SB.cantInodosLibres>0){
         SB.posPrimerInodoLibre=posInodoReservado+1;   
        } 

        if(bwrite(0,&SB)==-1){
            fprintf(stderr,"(reservar_inodo) --> Error escritura");
            return -1;
        }    

        return posInodoReservado;

    }else{
        fprintf(stderr,"(reservar_inodo) --> No hay inodos libres");
        return -1;
    }      

}
int obtener_nRangoBL(struct inodo *inodo, unsigned int nblogico, unsigned int *ptr){
 
    if(nblogico < DIRECTOS){      
        *ptr = inodo->punterosDirectos[nblogico];   
        return 0; 
    }else if(nblogico < INDIRECTOS0){         
        *ptr = inodo->punterosIndirectos[0];       
        return 1;     
    }else if(nblogico < INDIRECTOS1){           
        *ptr = inodo->punterosIndirectos[1];            
        return 2;       
    }else if(nblogico < INDIRECTOS2){              
        *ptr = inodo->punterosIndirectos[2];              
        return 3;          
    }else{         
        *ptr = 0; 
        fprintf(stderr,"(obtener_nRangoBL) --> Bloque lógico fuera de rango");                
        return -1;
    }      
}

int obtener_indice (int nblogico, int nivel_punteros){ 
 
    if(nblogico < DIRECTOS){ 
        return nblogico;   
    }else if(nblogico < INDIRECTOS0){ 
            return (nblogico - DIRECTOS) ; 
    }else if(nblogico < INDIRECTOS1){       
        if(nivel_punteros == 2){ 
            return ((nblogico - INDIRECTOS0) / NPUNTEROS);          
        }else if(nivel_punteros == 1){
           return ((nblogico - INDIRECTOS0) % NPUNTEROS);  
        }    
    }else if(nblogico < INDIRECTOS2){        
        if(nivel_punteros == 3){
            return ((nblogico - INDIRECTOS1) / (NPUNTEROS * NPUNTEROS));              
        }else if(nivel_punteros == 2){      
            return (((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) / NPUNTEROS);           
        }else if(nivel_punteros == 1){   
            return (((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) % NPUNTEROS);
        }          
    }
    fprintf(stderr, "(obtener_indice) --> No se ha encontrado el rango");
    return -1;
}

int traducir_bloque_inodo(unsigned int ninodo, unsigned int nblogico, char reservar){
    struct inodo inodos;
    int ptr_ant, salvar_inodo, nRangoBL, nivel_punteros, indice;
    unsigned int ptr;
    int buffer[NPUNTEROS];

    leer_inodo(ninodo, &inodos); 
    ptr = 0;
    ptr_ant = 0;
    salvar_inodo = 0;

    nRangoBL = obtener_nRangoBL(&inodos, nblogico, &ptr);
    nivel_punteros = nRangoBL;
    
    while (nivel_punteros > 0){
        if (ptr == 0){
            if (reservar == 0){
                return -1;
            }
            else{

                salvar_inodo = 1;
               
                ptr = reservar_bloque();
                inodos.numBloquesOcupados++;
                inodos.ctime = time(NULL);
                if (nivel_punteros == nRangoBL){
                    inodos.punterosIndirectos[nRangoBL - 1] = ptr;
#if DEBUGTraducir
                    printf("traducir_bloque_inodo()→ inodo.punterosIndirectos[%i] = %i (reservado BF %i para punteros_nivel%i)\n", nRangoBL
                    -1, ptr, ptr, nivel_punteros);
#endif
                }
                else{
                    
                    buffer[indice] = ptr;
#if DEBUGTraducir
                    printf("traducir_bloque_inodo()→ inodo.punteros_nivel%i[%i] = %i (reservado BF %i para punteros_nivel%i)\n", nivel_punteros, indice, ptr, ptr, nivel_punteros);
#endif
                    if (bwrite(ptr_ant, buffer) == -1){
                        fprintf(stderr, "Error escritura");
                        return -1;
                    }
                }
            }
        }

        if (bread(ptr, buffer) == -1){
            fprintf(stderr, "Error lectura");
            return -1;
        }

        indice = obtener_indice(nblogico, nivel_punteros);
        ptr_ant = ptr;
        ptr = buffer[indice];
        nivel_punteros--;
    }

    if (ptr == 0){
        if (reservar == 0){
            return -1;
        }
        else{

            salvar_inodo = 1;
            // printf("Reservar bloque: %d\n",reservar_bloque());
            ptr = reservar_bloque();
            //printf("Ptr2: %d\n",ptr);
            inodos.numBloquesOcupados++;
            inodos.ctime = time(NULL);

            if (nRangoBL == 0){
                inodos.punterosDirectos[nblogico] = ptr;
#if DEBUGTraducir
                printf("[traducir_bloque_inodo()→ inodo.punterosDirectos[%i] = %i (reservado BF %i para BL %i)]\n", nblogico, ptr, ptr, nblogico);
#endif
                //inodos.punterosDirectos[nblogico] = ptr;
            }
            else{
                buffer[indice] = ptr;
#if DEBUGTraducir
                printf("[traducir_bloque_inodo()→ inodo.punteros_nivel1[%i] = %i (reservado BF %i para BL %i)]\n", indice, ptr, ptr, nblogico);
#endif
                if (bwrite(ptr_ant, buffer) == -1){
                    fprintf(stderr, "Error escritura");
                    return -1;
                }
            }
        }
    }

    if (salvar_inodo == 1){
        escribir_inodo(ninodo, inodos);
    }
    
    return ptr;
}

int liberar_inodo(unsigned int ninodo)
{

    int liberados = 0;
    struct inodo inodo;

    if (leer_inodo(ninodo, &inodo) == -1)
    {
        fprintf(stderr, "liberar_inodo(): Error al leer_inodo()\n");
        return -1;
    }


    if (inodo.tamEnBytesLog > 0)
    {
        liberados = liberar_bloques_inodo(0, &inodo);
    }
    inodo.numBloquesOcupados =inodo.numBloquesOcupados - liberados;
    inodo.tipo = 'l';
    inodo.tamEnBytesLog = 0;

    struct superbloque SB;
    if (bread(posSB, &SB) == -1)
    {
        fprintf(stderr, "liberar_inodo(): Error al leer el SB.\n");
        return -1;
    }

    unsigned int auxinodo = SB.posPrimerInodoLibre;
    SB.posPrimerInodoLibre = ninodo;
    inodo.punterosDirectos[0] = auxinodo;
    SB.cantInodosLibres++;

    if (escribir_inodo(ninodo, inodo) == -1)
    {
        fprintf(stderr, "liberar_inodo(): Error escribir_inodo().\n");
        return -1;
    }

    if (bwrite(posSB, &SB) == -1)
    {
        fprintf(stderr, "liberar_inodo(): Error al escribir el SB.\n");
        return -1;
    }

    return ninodo;
}
int liberar_bloques_inodo(unsigned int primerBL,struct inodo *inodo) {
    
    unsigned int nivel_punteros,indice,ptr,nBL,ultimoBL;
    int nRangoBL;
    int bloques_punteros[3][NPUNTEROS];
    int ptr_nivel[3];
    int indices[3];
    int liberados;
    unsigned char bufAux_punteros[BLOCKSIZE];
    memset(bufAux_punteros,0,BLOCKSIZE);
    
    liberados = 0;
    if (inodo -> tamEnBytesLog == 0){ // el fichero está vacío
        return 0;
    }

    //obtenemos el último bloque lógico del inodo
    if ((inodo -> tamEnBytesLog % BLOCKSIZE) == 0) {
        ultimoBL = inodo -> tamEnBytesLog/BLOCKSIZE - 1;
    }else{
        ultimoBL = inodo -> tamEnBytesLog / BLOCKSIZE;
    }

    ptr = 0;
#if DEBUGLiberar
    fprintf(stderr, "[liberar_bloques_inodo() → primer BL: %i, último BL: %i]\n",primerBL, ultimoBL);
#endif

    for (nBL = primerBL ; nBL <= ultimoBL ; nBL++){ //iteramos para cada bloque lógico 
        nRangoBL = obtener_nRangoBL(inodo,nBL,&ptr); //obtenemos el rango del BL y ptr

        if (nRangoBL < 0){
            fprintf(stderr, "Error en el Rango\n"); 
            return -1;
        }

        nivel_punteros = nRangoBL; //el nivel_punteros +alto cuelga del inodo
        while (ptr > 0 && nivel_punteros > 0){ //cuelgan bloques de punteros
            
            indice = obtener_indice(nBL,nivel_punteros); //indice de bloques de punteros

            if(indice == 0 || nBL == primerBL){ //si es la primera vez que iteramos el bloque
                if (bread(ptr, bloques_punteros[nivel_punteros - 1]) == -1){ //solo leemos del dispositivo si no está ya cargado previamente en un buffer
                    fprintf(stderr, "Error lectura");
                    return -1;
                }
            }

            ptr_nivel[nivel_punteros - 1] = ptr;
            indices[nivel_punteros - 1] = indice;
            ptr = bloques_punteros[nivel_punteros - 1][indice];
            nivel_punteros -- ; 
        }

        if (ptr > 0){ //si existe bloque de datos
            liberar_bloque(ptr); 
            liberados++;
#if DEBUGLiberar
            fprintf(stderr, "[liberar_bloques_inodo()→ liberado BF %i de datos para BL: %i]\n",ptr, nBL);
#endif
            
            if (nRangoBL == 0){ 
                inodo -> punterosDirectos[nBL] = 0; //ponemos el espacio del BF de datos en Directos a 0
            }else{
                //nivel_punter =1;
                while (nivel_punteros < nRangoBL){ //liberamos cada inodo hasta el nivel del cual empezamos a buscar
                    
                    indice = indices[nivel_punteros]; //cojemos el indice guardado del correspondiente BF
                    bloques_punteros[nivel_punteros][indice] = 0; //ponemos a 0 su puntero dentro del bloque
                    ptr = ptr_nivel[nivel_punteros]; //cojemos el puntero al bloque de punteros
                    
                    if(memcmp(bloques_punteros[nivel_punteros],bufAux_punteros,BLOCKSIZE) == 0){ //Si no apunta a nada mas
                        
                        //No cuelgan más bloques ocupados, hay que liberar el bloque de punteros
                        liberar_bloque(ptr);
                        liberados ++;
                        //fprintf(stderr, "[liberar_bloques_inodo()→ liberado BF %i de punteros_nivel%i correspondiente al BL: %i]\n",ptr, nivel_punteros + 1, nBL);
                        nivel_punteros ++;
                        
                        //Ya hemos liberado toda una rama de punteros
                        if (nivel_punteros == nRangoBL){
                            inodo -> punterosIndirectos[nRangoBL - 1] = 0; //eliminamos el puntero del inodo
                        }

                    }else{ //escribimos en el dispositivo el bloque de punteros modificado
                        bwrite(ptr,bloques_punteros[nivel_punteros]);
                        // hemos de salir del bucle ya que no será necesario liberar los bloques de niveles
                        // superiores de los que cuelga
                        nivel_punteros = nRangoBL;   
                        
                    }
                }
            }
        }
    }
#if DEBUGLiberar
            fprintf(stderr, "[liberar_bloques_inodo() → total bloques liberados: %i]\n",liberados);
#endif
    
    return liberados;
}
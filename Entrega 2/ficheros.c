//Carlos Ecker, Pedro Marti y Alejandro Medina 
#include "ficheros.h"


int mi_write_f(unsigned int ninodo, const void *buf_original,unsigned int offset ,unsigned int nbytes){
    
    struct inodo inodo;
    unsigned char buf_bloque[BLOCKSIZE];
    int bytes_escritos = 0;
    leer_inodo(ninodo, &inodo);

    if((inodo.permisos & 2) == 2){
        int primerBLogico = offset/BLOCKSIZE;
        int ultimoBLogico = (offset + nbytes - 1) / BLOCKSIZE;
        int desp1 = offset % BLOCKSIZE;
        int desp2 = (offset + nbytes - 1) % BLOCKSIZE;
        
        int nbfisico = traducir_bloque_inodo(ninodo,primerBLogico,1);
        if(desp1 != 0){
            if(bread(nbfisico, buf_bloque)==-1){
                
                fprintf(stderr,"Error lectura en mi_write\n");
                return -1;
            }
        }
         if(nbfisico == -1){
             fprintf(stderr,"Error en la lectura del nbfisico");
             
             return -1;
            }
          
        if(bread(nbfisico, buf_bloque)==-1){
                fprintf(stderr,"Error lectura en mi_write\n");
                return -1;
            }

        if(primerBLogico == ultimoBLogico){
    
            memcpy(buf_bloque + desp1, buf_original,nbytes);
            bytes_escritos += nbytes;
            if(bwrite(nbfisico, buf_bloque)==-1){
                fprintf(stderr,"Error escritura en mi_write\n");
                return -1;
            }
        }
        else{
            desp2 = offset + nbytes - 1;
            memcpy (buf_bloque + desp1, buf_original, BLOCKSIZE - desp1);
            if(bwrite(nbfisico, buf_bloque)==-1){
                fprintf(stderr,"Error escritura en mi_write\n");
                return -1;
            }
            bytes_escritos += BLOCKSIZE - desp1;

            for(int i = primerBLogico + 1; i != ultimoBLogico; i ++){
                
                nbfisico = traducir_bloque_inodo(ninodo, i, 1);
         if(nbfisico == -1){
             fprintf(stderr,"Error en la lectura del nbfisico");
             
             return -1;
            }
                bytes_escritos += bwrite(nbfisico, (buf_original + (BLOCKSIZE - desp1) + (i - primerBLogico - 1) * BLOCKSIZE));
                
            }
            
            desp2 = desp2 % BLOCKSIZE;
            
            nbfisico = traducir_bloque_inodo(ninodo, ultimoBLogico, 1);
         if(nbfisico == -1){
             fprintf(stderr,"Error en la lectura del nbfisico");
             
             return -1;
            }
         
            if(bread(nbfisico, buf_bloque)==-1){
               
                fprintf(stderr,"Error lectura en mi_write\n");
                return -1;
            }
            
            memcpy (buf_bloque, buf_original + (nbytes - desp2 - 1), desp2 + 1);
            bytes_escritos += desp2 + 1;
            if(bwrite(nbfisico, buf_bloque)==-1){
                fprintf(stderr,"Error escritura en mi_write\n");
                return -1;
            }
        }
        int bytes_escritos2 = bytes_escritos + offset;
        //volvemos a leer el inodo, se han reservado bloques
        
        if (leer_inodo(ninodo, &inodo) == -1)
        {
            
            fprintf(stderr, "Error lectura inodo\n");
            return -1;
        }
        if(inodo.tamEnBytesLog < bytes_escritos2){
            inodo.tamEnBytesLog = bytes_escritos2;
            inodo.ctime = time(NULL);
        }
        inodo.mtime = time (NULL);
        escribir_inodo(ninodo, inodo);
        
        return bytes_escritos; 
    }
    else{
        fprintf(stderr, "\n¡No hay permisos de escritura!\n");
        return -1;
    }
}


int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes)
{
    struct inodo inodo;
    
    if (leer_inodo(ninodo, &inodo) == -1)
    {
        
        fprintf(stderr, "Error lectura inodo\n");
        return -1;
    }
    inodo.atime = time(NULL);
    escribir_inodo(ninodo,inodo);
    

    if ((inodo.permisos & 4) != 4)
    {
        fprintf(stderr, "No hay permisos de lectura sobre el inodo\n");
        return -1;
    }


    if (offset >= inodo.tamEnBytesLog)
    {
        return 0;

    }

    if ((offset + nbytes) >= inodo.tamEnBytesLog)
    {

        nbytes = inodo.tamEnBytesLog - offset;
    }
 
    int primerBL = offset / BLOCKSIZE;
    int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
    int nbytesLeidosReal = 0;
    int desp1 = offset % BLOCKSIZE;
    int desp2 = (offset + nbytes - 1) % BLOCKSIZE;
    char buf_bloque[BLOCKSIZE];

    if (primerBL == ultimoBL)
    {
        int nnbfisico = traducir_bloque_inodo(ninodo, primerBL, 0);
        if (nnbfisico != -1)
        {
            if (bread(nnbfisico, buf_bloque) == -1)
            {
                fprintf(stderr, "Error de lectura en bloque físico\n");
                return -1;
            }
            memcpy(buf_original, buf_bloque + desp1, nbytes);
        }

        nbytesLeidosReal = nbytes;
    }

    else
    {

        int nnbfisico = traducir_bloque_inodo(ninodo, primerBL, 0);
        if (nnbfisico != -1)
        {
            if (bread(nnbfisico, buf_bloque) == -1)
            {
                fprintf(stderr, "Error de lectura en bloque físico\n");
                return -1;
            }

            memcpy(buf_original, buf_bloque + desp1, BLOCKSIZE - desp1);
        }

        nbytesLeidosReal = nbytesLeidosReal + (BLOCKSIZE - desp1);

        for (int i = primerBL + 1; i < ultimoBL; i++)
        {
            nnbfisico = traducir_bloque_inodo(ninodo, i, 0);
            if (nnbfisico != -1)
            {
                if (bread(nnbfisico, buf_bloque) == -1)
                {
                    fprintf(stderr, "Error de lectura en bloque físico\n");
                    return -1;
                }

                memcpy(buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE, buf_bloque, BLOCKSIZE);
            }
            nbytesLeidosReal = nbytesLeidosReal + BLOCKSIZE;
        }

        nnbfisico = traducir_bloque_inodo(ninodo, ultimoBL, 0);
        if (nnbfisico != -1)
        {
            if (bread(nnbfisico, buf_bloque) == -1)
            {
                fprintf(stderr, "Error de lectura en bloque físico\n");
                return -1;
            }
            memcpy(buf_original + (nbytes - desp2 - 1), buf_bloque, desp2 + 1);
        }
        nbytesLeidosReal = nbytesLeidosReal + (desp2 + 1);

    }
    return nbytesLeidosReal;
}


int mi_stat_f(unsigned int ninodo, struct STAT *p_stat)
{
    //stat examina el fichero al que apunta file_name y llena buf
    //truct STAT es igual que el struct INODO pero sin los punteros, no requiere de padding
    struct inodo inodos;
    leer_inodo(ninodo, &inodos);                            //Cogemos ninodo (fichero/directorio) y lo pasamos a inodo
    p_stat->tipo = inodos.tipo;                             //tipo
    p_stat->permisos = inodos.permisos;                     //permisos
    p_stat->nlinks = inodos.nlinks;                         //cantidad de enlaces de entradas en directorio
    p_stat->tamEnBytesLog = inodos.tamEnBytesLog;           //tamaño en bytes lógicos
    p_stat->atime = inodos.atime;                           //timestamps
    p_stat->ctime = inodos.ctime;                           //timestamps
    p_stat->mtime = inodos.mtime;                           //timestamps
    p_stat->numBloquesOcupados = inodos.numBloquesOcupados; //cantidad de bloques ocupados
    return 0;
}

int mi_chmod_f(unsigned int ninodo, unsigned char permisos){

    
    struct inodo inodo;
    
    if (leer_inodo(ninodo, &inodo) == -1){
                         //Leemos el inodo que nos dice "ninodo"
        fprintf(stderr,"(ficheros.c) [mi_chmod_f] --> Error leyendo el inodo");
        return -1;
    }

    inodo.permisos = permisos; //Pasamos los permisos (del arg) al inodo, actualizandolo
    inodo.ctime = time(NULL);  //Actualizamos ctime

    if (escribir_inodo(ninodo, inodo) == -1){
             //Sobreescribimos el inodo con los permisos actulizados
        fprintf(stderr,"(ficheros.c) [mi_chmod_f] --> Error escribiendo el inodo");
        return -1;
    }

    
    return 0;
}

int mi_truncar_f(unsigned int ninodo, unsigned int nbytes){

    struct inodo inodo;
    int liberados;
    unsigned int primerBL;

    if (nbytes % BLOCKSIZE == 0){
        primerBL = nbytes / BLOCKSIZE;
    }else{
        primerBL = nbytes / BLOCKSIZE + 1;
    }

    if(leer_inodo(ninodo, &inodo) == -1){
        fprintf(stderr,"(ficheros.c) [mi_chmod_f] --> Error leyendo el inodo");
        return -1;
    }

    liberados = liberar_bloques_inodo(primerBL, &inodo);

    inodo.mtime = time(NULL);
    inodo.ctime = time(NULL);
    inodo.tamEnBytesLog = nbytes;
    inodo.numBloquesOcupados = inodo.numBloquesOcupados - liberados;
    escribir_inodo(ninodo,inodo);
    return liberados;
}
    
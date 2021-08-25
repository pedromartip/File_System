//Carlos Ecker, Pedro Marti y Alejandro Medina 
#include "bloques.h"
#include <limits.h>
#include <time.h>

#define posSB 0
#define tamSB 1
#define INODOSIZE 128
#define NPUNTEROS (BLOCKSIZE / sizeof(unsigned int)) // 256
#define DIRECTOS 12
#define INDIRECTOS0 (NPUNTEROS + DIRECTOS)                            // 268
#define INDIRECTOS1 (NPUNTEROS * NPUNTEROS + INDIRECTOS0)             // 65.804
#define INDIRECTOS2 (NPUNTEROS * NPUNTEROS * NPUNTEROS + INDIRECTOS1) // 16.843.020

struct superbloque{
    unsigned int posPrimerBLoqueMB;
    unsigned int posUltimoBLoqueMB;
    unsigned int posULtimoBLoqueAI;
    unsigned int posUltimoBloqueDatos;
    unsigned int posPrimerBLoqueAI;
    unsigned int posPrimerBLoqueDatos;
    unsigned int posInodoRaiz;

    unsigned int posPrimerInodoLibre;
    unsigned int cantInodosLibres;
    unsigned int cantBloquesLibres;
    unsigned int totBloques;
    unsigned int totInodos;
    
    char padding[BLOCKSIZE - 12 * sizeof(unsigned int)];

};
struct superbloque SB;

struct inodo{
    unsigned char tipo;
    unsigned char permisos;
    unsigned char reservado_alineacion1[6];
    time_t atime;
    time_t mtime;
    time_t ctime;
    unsigned int nlinks;
    unsigned int tamEnBytesLog;
    unsigned int numBloquesOcupados;
    unsigned int punterosDirectos[12];
    unsigned int punterosIndirectos[3];
    char padding[INODOSIZE-2*sizeof(unsigned char)-3*sizeof(time_t)
                -18*sizeof(unsigned int)-6*sizeof(unsigned char)];
};


int tamMB(unsigned int nbloques);
int tamAI(unsigned int x);
int initSB(unsigned int nbloques, unsigned int ninodos);
int initMB();
int initAI();
int escribir_bit(unsigned int nbloque, unsigned int bit);
char leer_bit (unsigned int nbloque);
int reservar_bloque();
int liberar_bloque (unsigned int nbloque);
int escribir_inodo(unsigned int ninodo, struct inodo inodo);
int leer_inodo(unsigned int ninodo, struct inodo *inodo);
int reservar_inodo(unsigned char tipo, unsigned char permisos);
int obtener_nRangoBL(struct inodo *inodo, unsigned int nblogico, unsigned int *ptr);
int obtener_indice (int nblogico, int nivel_punteros);
int traducir_bloque_inodo(unsigned int ninodo, unsigned int nblogico, char reservar);
int liberar_inodo(unsigned int ninodo);
int liberar_bloques_inodo(unsigned int primerBL, struct inodo *inodo);
int mi_truncar_f(unsigned int ninodo, unsigned int nbytes);
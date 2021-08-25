//Carlos Ecker, Pedro Marti y Alejandro Medina 
#include "directorios.h"

int main(int argc, char **argsv){
  char descrpitor [1024];
  strcpy(descrpitor,argsv[1]);
  bmount(descrpitor);

    bread(posSB, &SB);

    printf("DATOS DEL SUPERBLOQUE");
    printf("\nposPrimerBloqueMB = %d",SB.posPrimerBLoqueMB);
    printf("\nposUltimoBloqueMB = %d ",SB.posUltimoBLoqueMB);
    printf("\nposPrimerBloqueAI = %d",SB.posPrimerBLoqueAI);
    printf("\nposUltimoBloqueAI = %d",SB.posULtimoBLoqueAI);
    printf("\nposPrimerBloqueDatos = %d",SB.posPrimerBLoqueDatos);
    printf("\nposUltimoBloqueDatos = %d",SB.posUltimoBloqueDatos);
    printf("\nposInodoRaiz = %d",SB.posInodoRaiz);
    printf("\nposPrimerInodoLibre = %d",SB.posPrimerInodoLibre);
    printf("\ncantBloquesLibres = %d",SB.cantBloquesLibres);
    printf("\ncantInodosLibres = %d",SB.cantInodosLibres);
    printf("\ntotBloques = %d",SB.totBloques);
    printf("\ntotInodos= %d\n",SB.totInodos);


    
  bumount();
  return 0;
    
    
}

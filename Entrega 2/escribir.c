//Carlos Ecker, Pedro Marti y Alejandro Medina 
#include "ficheros.h"

int main (int argc, char *argv[]){



    if (argc != 4)
    {
        fprintf(stderr, "Error Sintaxis: escribir <nombre_dispositivo> <\"$(cat fichero)\"> <diferentes_inodos>\n");
        return -1;
    }

    printf("longitud texto: %ld\n\n", strlen(argv[2]));
    int offsets[5] = {9000, 209000, 30725000, 409605000, 480000000};
    char descriptor[1024];

    strcpy(descriptor, argv[1]);
    if (bmount(descriptor) == -1)
    {
        fprintf(stderr, "escribir.c: Error al montar el dispositivo virtual.\n");
        return -1;
    }

    int ninodo = reservar_inodo('f', 6);
    if (ninodo == -1)
    {
        fprintf(stderr, "escribir.c: Error al reservar el inodo.\n");
        return -1;
    }

    for (int i = 0; i < (sizeof(offsets) / sizeof(int)); i++)
    {

        printf("Nº inodo reservado: %d\n", ninodo);
        printf("offset: %d\n", offsets[i]);
        int bytesescritos = mi_write_f(ninodo, argv[2], offsets[i], strlen(argv[2]));
        if (bytesescritos == -1)
        {
            fprintf(stderr, "escribir.c: Error mi_write_f().\n");
            return -1;
        }
        printf("Bytes escritos: %d\n", bytesescritos);

        struct STAT st;

        if (mi_stat_f(ninodo, &st))
        {
            fprintf(stderr, "escribir.c: Error mi_stat_f()\n");
            return EXIT_FAILURE;
        }

        printf("stat.tamEnBytesLog = %d\n", st.tamEnBytesLog);
        printf("stat.numBloquesOcupados = %d\n\n", st.numBloquesOcupados);
        
        if (strcmp(argv[3], "0"))
        {
            if (reservar_inodo('f', 6)== -1)
            {
                fprintf(stderr, "escribir.c: Error al reservar el inodo.\n");
                return -1;
            }
        }
    }

    if (bumount() == -1)
    {
        fprintf(stderr, "escribir.c: Error al desmonta el dispositivo virtual.\n");
        return -1;
    }
    return 0;
}

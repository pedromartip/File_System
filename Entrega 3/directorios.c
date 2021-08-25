//Carlos Ecker, Pedro Marti y Alejandro Medina 
#include "directorios.h"
#define TAMFILA 100

static struct UltimaEntrada UltimaEntradaEscritura;
static struct UltimaEntrada UltimaEntradaLectura;
#define DebugBuscarEntrada 0

int extraer_camino(const char *camino, char *inicial, char *final, char *tipo)
{

    //  DOS CASOS:
    //     - CASO 1 --> Hay un segundo '/': Guarda en *inicial la porción de *camino comprendida entre los dos primeros '/'.
    //     CONTENDRÀ UN DIRECTORIO
    //     - CASO 2 --> No hay un segundo '/': Guarda camino (todo) en inicial, excepto la priemra '/'.

    int encontrado = 0;
    if (camino[0] == '/')
    {
        for (int i = 1; i < strlen(camino) && encontrado == 0; i++)
        {
            if (camino[i] == '/')
            { //caso en el que le pasemos //algo.txt no gestionado(doble barra)
                strncpy(inicial, camino + 1, sizeof(char) * i - 1);
                encontrado = 1;
                strcpy(final, camino + i);
                strcpy(tipo, "d");
                break;
            }
        }

        if (encontrado == 0)
        {
            strncpy(inicial, camino + 1, sizeof(char) * strlen(camino) - 1); //+1 para evitar la primera '/'
            strcpy(tipo, "f");
        }
        return 0;
    }

    else
    {
        return -1;
    }
}

int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos)
{
    //reservamos variables
    struct entrada entrada;
    struct inodo inodo_dir;
    char inicial[sizeof(entrada.nombre)];
    memset(inicial, 0, sizeof(inicial));
    char final[strlen(camino_parcial) + 1]; //cambio
    memset(final, 0, sizeof(final));
    char tipo;
    int cant_entradas_inodo, num_entrada_inodo;

    if (bread(posSB, &SB) == -1)
    { // Leer el SuperBloque para tener los valores actuales
        fprintf(stderr, "Error lectura \n");
        return -1;
    }

    //camino_parcial es "/"
    if (camino_parcial[0] == '/' && strlen(camino_parcial) == 1)
    {
        *p_inodo = SB.posInodoRaiz;
        *p_entrada = 0;
        return 0;
    }

    if (extraer_camino(camino_parcial, inicial, final, &tipo) == -1)
    {
        return ERROR_CAMINO_INCORRECTO;
    }
#if DebugBuscarEntrada
    fprintf(stderr, "[buscar_entrada() → inicial: %s, final: %s, reserva: %d] \n", inicial, final, reservar);
#endif
    leer_inodo(*p_inodo_dir, &inodo_dir);
    if ((inodo_dir.permisos & 4) != 4)
    {
        return ERROR_PERMISO_LECTURA;
    }

    memset(entrada.nombre, 0, sizeof(entrada.nombre));
    struct entrada entradas[BLOCKSIZE / sizeof(struct entrada)];
    memset(entradas, 0, sizeof(entradas));
    cant_entradas_inodo = inodo_dir.tamEnBytesLog / sizeof(struct entrada);
    num_entrada_inodo = 0;
    //calculamos número de entradas a inodo

    if (cant_entradas_inodo > 0)
    {
        int offset = 0;
        offset += mi_read_f(*p_inodo_dir, entradas, offset, BLOCKSIZE);

        while ((num_entrada_inodo < cant_entradas_inodo) && (strcmp(inicial, entradas[num_entrada_inodo % (BLOCKSIZE / sizeof(struct entrada))].nombre) != 0))
        {

            num_entrada_inodo++;
            // fprintf(stderr,"num_inodo: %i\tcantidad_entradas: %i\n", num_entrada_inodo, cant_entradas_inodo);
            if ((num_entrada_inodo % (BLOCKSIZE / sizeof(struct entrada))) == 0)
            {
                memset(entradas, 0, sizeof(entradas));
                offset += mi_read_f(*p_inodo_dir, entradas, offset, BLOCKSIZE);
            }
        }
        memcpy(&entrada, &entradas[num_entrada_inodo % (BLOCKSIZE / sizeof(struct entrada))], sizeof(struct entrada));
    }

    // fprintf(stderr, "nombre : %s\n", buf_entradas[num_entrada_inodo].nombre);
    if ((strcmp(inicial, entradas[num_entrada_inodo % (BLOCKSIZE / sizeof(struct entrada))].nombre)) != 0)
    { 

        switch (reservar)
        {

        case 0:
            return ERROR_NO_EXISTE_ENTRADA_CONSULTA;
        case 1:
            if (inodo_dir.tipo == 'f')
            {
                return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;
            }

            if ((inodo_dir.permisos & 2) != 2)
            {
                return ERROR_PERMISO_ESCRITURA;
            }
            else
            {
                strcpy(entrada.nombre, inicial);
                if (tipo == 'd')
                {
                    if (strcmp(final, "/") == 0)
                    {
                        entrada.ninodo = reservar_inodo('d', permisos);
#if DebugBuscarEntrada
                        fprintf(stderr, "[buscar_entrada() → reservado inodo: %d tipo 'd' con permisos %u para: %s]\n", entrada.ninodo, permisos, entrada.nombre);
#endif
                        
                    }
                    else
                    {
                        // printf(" ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO (BUSCAR_ENTRADA, DIRECTORIOS.C)");
                        return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                    }
                }
                else
                {
                    entrada.ninodo = reservar_inodo('f', permisos);
#if DebugBuscarEntrada
                        fprintf(stderr, "[buscar_entrada() → reservado inodo: %d tipo 'f' con permisos %u para: %s]\n", entrada.ninodo, permisos, entrada.nombre);
#endif
                }

                if (mi_write_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) == -1)
                {

                    if (entrada.ninodo != -1)
                    {   
#if DebugBuscarEntrada
                        fprintf(stderr, "[buscar_entrada() → liberado inodo %i, reservado a %s\n", num_entrada_inodo, inicial);
#endif
                    
                        liberar_inodo(entrada.ninodo);
                    }

                    return -1;
                }
            }
        }
    }
    if (((strcmp(final, "/")) == 0) || ((strcmp(final, "")) == 0))
    {
        if ((num_entrada_inodo < cant_entradas_inodo) && (reservar == 1))
        {

            return ERROR_ENTRADA_YA_EXISTENTE;
        }

        *p_inodo = entrada.ninodo;
        *p_entrada = num_entrada_inodo;
        return 0;
    }
    else
    {
        *p_inodo_dir = entrada.ninodo;

        return buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
    }

    return 0;
}

void mostrar_error_buscar_entrada(int error)
{
    // fprintf(stderr, "Error: %d\n", error);
    switch (error)
    {
    case -1:
        fprintf(stderr, "Error: Camino incorrecto.\n");
        break;
    case -2:
        fprintf(stderr, "Error: Permiso denegado de lectura.\n");
        break;
    case -3:
        fprintf(stderr, "Error: No existe el archivo o el directorio.\n");
        break;
    case -4:
        fprintf(stderr, "Error: No existe algún directorio intermedio.\n");
        break;
    case -5:
        fprintf(stderr, "Error: Permiso denegado de escritura.\n");
        break;
    case -6:
        fprintf(stderr, "Error: El archivo ya existe.\n");
        break;
    case -7:
        fprintf(stderr, "Error: No es un directorio.\n");
        break;
    }
}

int mi_creat(const char *camino, unsigned char permisos)
{

    mi_waitSem();
    //En la función buscar_entrada() con reservar = 1 y que p_inodo_dir es 0).
    bread(posSB, &SB);
    unsigned int p_inodo_dir = SB.posInodoRaiz, p_inodo = 0, p_entrada = 0;
    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 1, permisos);

    if (error < 0)
    {
        mi_signalSem();
        mostrar_error_buscar_entrada(error);
        return -1;
    }
    else
    {

        mi_signalSem();
        return 0;
    }
}
//pone el contenido del directorio en un buffer de memoria y devuelve el número de entradas
int mi_dir(const char *camino, char *buffer, char *tipo)
{

    bread(0, &SB);
    unsigned int p_inodo_dir = SB.posInodoRaiz;
    struct inodo inodo;
    unsigned int ninodo = 0;
    unsigned int p_inodo, inicial = 0;
    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &inicial, 0, '4');
    if (error < 0)
    {
        return -1;
    }
    if (leer_inodo(p_inodo, &inodo) == -1)
    {
        fprintf(stderr, "Error en directorios.c mi_dir() --> No se ha podido leer el inodo %d\n", ninodo);
        return -1;
    }
    *tipo = inodo.tipo;
    // Comprobamos errores
    if ((inodo.permisos & 2) != 2)
    {
        fprintf(stderr, "El fichero '%s' no tiene permisos de lectura.\n", camino);
        return -1;
    }
    if (inodo.tipo != 'd')
    {
        fprintf(stderr, "%s no es un directorio!\n", camino);
        return -1;
    }
    // Creamos la cadena de salida
    struct inodo inodos;
    struct tm *tm;
    unsigned int numEntradas = inodo.tamEnBytesLog / sizeof(struct entrada);

    struct entrada Buffer_entradas[numEntradas];

    char tmp[100];
    char tam[TAMFILA];
    error = leer_inodo(p_inodo, &inodos);
    if (error < 0)
    {
        return error;
    }
    for (int i = 0; i < numEntradas; i++)
    {
        if (mi_read_f(p_inodo, &Buffer_entradas[i], i * sizeof(struct entrada), sizeof(struct entrada)) < 0)
        {
            fprintf(stderr, "Error en directorios.c mi_dir()  --> No se ha podido leer la entrada %d\n", Buffer_entradas[i].ninodo);
            return -1;
        }
        if (leer_inodo(Buffer_entradas[i].ninodo, &inodos) < 0)
        {
            fprintf(stderr, "Error en directorios.c mi_dir()  --> No se ha podido leer la entrada %d\n", Buffer_entradas[i].ninodo);
            return -1;
        }

        if (inodos.tipo == 'd')
        {
            strcat(buffer, "d");
        }
        else
        {
            strcat(buffer, "f");
        }
        strcat(buffer, "\t");

        if (inodos.permisos & 4)
            strcat(buffer, "r");
        else
            strcat(buffer, "-");
        if (inodos.permisos & 2)
            strcat(buffer, "w");
        else
            strcat(buffer, "-");
        if (inodos.permisos & 1)
            strcat(buffer, "x");
        else
            strcat(buffer, "-");
        strcat(buffer, "\t");
        tm = localtime(&inodos.mtime);
        sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
        strcat(buffer, tmp);
        sprintf(tam, "\t%d\t", inodos.tamEnBytesLog);
        strcat(buffer, tam);
        strcat(buffer, Buffer_entradas[i].nombre);
        strcat(buffer, "\n");
    }
 
    return 0;
}

int mi_chmod(const char *camino, unsigned char permisos)
{

    bread(posSB, &SB);
    unsigned int p_inodo_dir = SB.posInodoRaiz;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;

    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 7);
    if (error < 0)
    {
        mostrar_error_buscar_entrada(error);
        return -1;
    }

    mi_chmod_f(p_inodo, permisos);
    return 0;
}

int mi_stat(const char *camino, struct STAT *p_stat)
{
    bread(posSB, &SB);
    unsigned int p_inodo_dir = SB.posInodoRaiz;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 7);
    if (error < 0)
    {
        mostrar_error_buscar_entrada(error);
        return -1;
    }
    printf("N inodo: %d\n", p_inodo);

    mi_stat_f(p_inodo, p_stat);

    return 0;
}
//Función para escribir contenido en un fichero
int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes)
{
 
    int numBytesEscritos = 0;
    bread(posSB, &SB);
    unsigned int p_inodo_dir = SB.posInodoRaiz;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;

    //Comprobaríamos si la escritura es sobre el mismo inodo:
    if (strcmp(UltimaEntradaEscritura.camino, camino) == 0)
    {
        p_inodo = UltimaEntradaEscritura.p_inodo;
    }
    else
    {
        // si no llamar a buscar_entrada() y actualizar los campos de UltimaEntradaEscritura
        // con el p_inodo obtenido para el camino buscado.
        int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4); 
        if (error < 0)
        {
            mostrar_error_buscar_entrada(error);
            return -1;
        }
        strcpy(UltimaEntradaEscritura.camino, camino);
        UltimaEntradaEscritura.p_inodo = p_inodo;
    }

    numBytesEscritos = mi_write_f(p_inodo, buf, offset, nbytes);
    return numBytesEscritos;
}

int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes)
{

    bread(posSB, &SB);
    unsigned int p_inodo_dir = SB.posInodoRaiz;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int numBytesLeidos;

    //Comprobaríamos si la escritura es sobre el mismo inodo:
    if (strcmp(UltimaEntradaLectura.camino, camino) == 0)
    {
        p_inodo = UltimaEntradaLectura.p_inodo;
    }
    else
    {
        /*si no llamar a buscar_entrada() y actualizar los campos de UltimaEntradaEscritura 
         con el p_inodo obtenido para el camino buscado. */
        int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4); //CAMBIADO LINIA 402
        if (error < 0)
        {
            mostrar_error_buscar_entrada(error);
            return -1;
        }
        strcpy(UltimaEntradaLectura.camino, camino);
        UltimaEntradaLectura.p_inodo = p_inodo;
    }

    numBytesLeidos = mi_read_f(p_inodo, buf, offset, nbytes);
    return numBytesLeidos;
}

int mi_link(const char *camino1, const char *camino2)
{
    mi_waitSem();

    bread(posSB, &SB);
    unsigned int p_inodo_dir1 = SB.posInodoRaiz;
    unsigned int p_inodo1 = 0, p_entrada1 = 0;
    unsigned int p_inodo_dir2 = SB.posInodoRaiz;
    unsigned int p_inodo2 = 0, p_entrada2 = 0;
    struct inodo inodos;
    struct entrada entrada;
    /* Hay que comprobar que la entrada camino1 exista. 
    Obtener el nº de inodo asociado, p_inodo1, mediante 
    la función buscar_entrada() y comprobar que tiene 
    permiso de lectura.  */
    int error = buscar_entrada(camino1, &p_inodo_dir1, &p_inodo1, &p_entrada1, 0, 4);
    if (error < 0)
    {
        mi_signalSem();
        mostrar_error_buscar_entrada(error);
        return -1;
    }
    leer_inodo(p_inodo1, &inodos);
    if (inodos.tipo != 'f')
    {
        mi_signalSem();
        fprintf(stderr, "El camino no se corresponde a un fichero");
        return -1;
    }
    if ((inodos.permisos & 4) != 4)
    {   
        mi_signalSem();
        fprintf(stderr, "No tiene permisos de lectura");
        return ERROR_PERMISO_LECTURA;
    }
    error = buscar_entrada(camino2, &p_inodo_dir2, &p_inodo2, &p_entrada2, 1, 6);
    if (error < 0)
    {
        mi_signalSem();
        mostrar_error_buscar_entrada(error);
        return -1;
    }

    if (mi_read_f(p_inodo_dir2, &entrada, (sizeof(struct entrada) * p_entrada2), sizeof(struct entrada)) == -1)
    {
        mi_signalSem();
        fprintf(stderr, "(directorios.c) [mi_link]--> Error: No se ha podido leer la entrada de ruta a enlazar\n");
        return -1;
    }

    entrada.ninodo = p_inodo1;
    if (mi_write_f(p_inodo_dir2, &entrada, (sizeof(struct entrada) * p_entrada2), sizeof(struct entrada)) == -1)
    {
        mi_signalSem();
        fprintf(stderr, "(directorios.c) [mi_link]--> Error escritura\n");
        return -1;
    }

    if (liberar_inodo(p_inodo2) == -1)
    {
        mi_signalSem();
        fprintf(stderr, "(directorios.c) [mi_link]--> No se ha podido liberar el inodo");
        return -1;
    }

    inodos.nlinks++;
    inodos.ctime = time(NULL);

    if (escribir_inodo(p_inodo1, inodos) == -1)
    {
        mi_signalSem();
        fprintf(stderr, "(directorios.c) [mi_link]--> Error de escritura inodo.\n");
        return -1;
    }

    mi_signalSem();
    return 0;
}

int mi_unlink(const char *camino)
{
    mi_waitSem(); 

    bread(posSB, &SB);
    unsigned int p_inodo_dir = SB.posInodoRaiz;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    struct inodo inodos;
    struct inodo inodo_dir;
    struct entrada entrada;

    //Hay que comprobar que la entrada camino exista y obtener su nº de entrada
    //(p_entrada), mediante la función buscar_entrada().
    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 6);

    if (error < 0)
    {
        mi_signalSem();
        mostrar_error_buscar_entrada(error);
        return -1;
    }

    if (leer_inodo(p_inodo, &inodos) == -1)
    {
        mi_signalSem();
        fprintf(stderr, "(directorios.c) [mi_link]--> Error de lectura del inodo\n");
        return -1;
    }

    if (inodos.tipo == 'd' && inodos.tamEnBytesLog > 0)
    {
        mi_signalSem();
        fprintf(stderr, "(directorios.c) [mi_link]--> Error: El directorio no esta vacio\n");
        return -1;
    }
    else
    {

        if (leer_inodo(p_inodo_dir, &inodo_dir) == -1)
        {
            mi_signalSem();
            fprintf(stderr, "(directorios.c) [mi_link]--> Error de lectura inodo\n");
            return -1;
        }

        int numEntradas = inodo_dir.tamEnBytesLog / sizeof(struct entrada);
        if (p_entrada == (numEntradas - 1))
        {
            if (mi_truncar_f(p_inodo_dir, (inodo_dir.tamEnBytesLog - sizeof(struct entrada))) == -1)
            {
                mi_signalSem();
                fprintf(stderr, "directorios.c) [mi_unlink]--> Error: No se ha podido truncar el inodo\n");
                return -1;
            }
        }
        else
        {
            if (mi_read_f(p_inodo_dir, &entrada, (sizeof(struct entrada) * (numEntradas - 1)), sizeof(struct entrada)) == -1)
            {
                mi_signalSem();
                fprintf(stderr, "(directorios.c) [mi_unlink]--> Error lectura\n");
                return -1;
            }

            if (mi_write_f(p_inodo_dir, &entrada, (sizeof(struct entrada) * p_entrada), sizeof(struct entrada)) == -1)
            {
                mi_signalSem();
                fprintf(stderr, "(directorios.c) [mi_unlink]--> Error lectura\n");
                return -1;
            }

            if (mi_truncar_f(p_inodo_dir, ((numEntradas - 1) * sizeof(struct entrada))) == -1)
            {
                mi_signalSem();
                fprintf(stderr, "directorios.c) [mi_unlink]--> Error: No se ha podido truncar el inodo\n");
                return -1;
            }
        }
        inodos.nlinks--;
        if (inodos.nlinks == 0)
        {
            if (liberar_inodo(p_inodo) == -1)
            {
                mi_signalSem();
                fprintf(stderr, "directorios.c) [mi_unlink]--> Error: Inodo no liberado\n");
                return -1;
            }
        }
        else
        {
            inodos.ctime = time(NULL);
            if (escribir_inodo(p_inodo, inodos) == -1)
            {
                mi_signalSem();
                fprintf(stderr, "directorios.c) [mi_unlink]--> Error escritura inodo\n");
                return -1;
            }
        }
    }

    mi_signalSem();
    return 0;
}
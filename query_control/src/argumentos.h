#ifndef ARGUMENTOS_H
#define ARGUMENTOS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    char* archivo_config;
    char* archivo_query; 
    char* prioridad;
} t_argumentos;

t_argumentos* procesar_argumentos(int argc, char** argv);



#endif
#include "main.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

t_argumentos* procesar_argumentos(int argc, char** argv)
{
    if (argc < 4) {
        fprintf(stderr, "Uso: %s [archivo_config] [archivo_query] [prioridad]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    t_argumentos* args = malloc(sizeof(t_argumentos));
    if (!args) {
        fprintf(stderr, "Error: No se pudo asignar memoria para argumentos\n");
        exit(EXIT_FAILURE);
    }
    
    args->archivo_config = strdup(argv[1]);
    args->archivo_query = strdup(argv[2]);
    args->prioridad = strdup(argv[3]);
    
    return args;
}
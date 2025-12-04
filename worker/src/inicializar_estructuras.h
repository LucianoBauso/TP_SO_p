#ifndef INICIALIZAR_ESTRUCTURAS_H_
#define INICIALIZAR_ESTRUCTURAS_H_

#include "worker.h"
#include <stdlib.h>

//---------------variables
extern t_log* logger;
extern t_config* config;

extern char * ip_storage;
extern char* puerto_storage;

extern char * ip_master;
extern char* puerto_master;

extern int retardo_memoria;
extern char* path_queries;



//-------------prototipos de funciones
t_log* iniciar_logger(void);
t_config* iniciar_config(char* );
void leer_config(t_config* config);
void inicializar_worker(char* path_config);

#endif
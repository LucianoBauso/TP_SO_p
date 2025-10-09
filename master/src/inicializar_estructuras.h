#ifndef INICIALIZAR_ESTRUCTURAS_H_
#define INICIALIZAR_ESTRUCTURAS_H_

#include <main.h>
#include <stdlib.h>

//---------------variables
extern t_log* logger;
extern t_config* config;

//--Variables que se usan a lo largo de la ejecucion del modulo. 
//extern char* coso;

//-------------prototipos de funciones
t_log* iniciar_logger(void);
t_config* iniciar_config(char*);
void leer_configStorage(t_config* configStorage);
void leer_configSuperBlock(t_config* configSuperBlock);
void inicializar_storage(void);

#endif
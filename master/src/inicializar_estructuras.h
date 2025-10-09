#ifndef INICIALIZAR_ESTRUCTURAS_H_
#define INICIALIZAR_ESTRUCTURAS_H_

#include <main.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>

//---------------variables
extern t_log* logger;
extern t_config* config;

//--Variables que se usan a lo largo de la ejecucion del modulo. 
//extern char* coso;

//-------------prototipos de funciones
t_log* iniciar_logger(void);
t_config* iniciar_config(char*);
void leer_config(t_config* config);
void inicializar_master(void);

#endif
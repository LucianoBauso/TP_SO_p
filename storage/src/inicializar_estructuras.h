#ifndef INICIALIZAR_ESTRUCTURAS_H_
#define INICIALIZAR_ESTRUCTURAS_H_

#include "storage.h"
#include <stdlib.h>

//---------------variables
extern t_log* logger;
extern t_config* config;



//-------------prototipos de funciones
t_log* iniciar_logger(void);
t_config* iniciar_config(void);
void leer_config(t_config* config);
void inicializar_storage(void);

#endif
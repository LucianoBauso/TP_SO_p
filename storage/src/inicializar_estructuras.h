#ifndef INICIALIZAR_ESTRUCTURAS_H_
#define INICIALIZAR_ESTRUCTURAS_H_

#include <storage.h>
#include <stdlib.h>

//---------------variables
extern t_log* logger;
extern t_config* configStorage;
extern t_config* configSuperBlock;

extern char* puerto_escucha;
extern int BLOCK_SIZE; // PREGUNTAR que onda el tipo ¿ 

//-------------prototipos de funciones
t_log* iniciar_logger(void);
t_config* iniciar_config(char*);
void leer_configStorage(t_config* configStorage);
void leer_configSuperBlock(t_config* configSuperBlock);
void inicializar_storage(void);
#endif
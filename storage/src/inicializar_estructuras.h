#ifndef INICIALIZAR_ESTRUCTURAS_H_
#define INICIALIZAR_ESTRUCTURAS_H_

#include <commons/log.h>
#include <commons/config.h>
#include <stdbool.h>

extern t_log* logger;
extern t_config* configStorage;
extern t_config* configSuperBlock;

extern char* puerto_escucha;
extern char* punto_montaje;
extern bool  fresh_start;
extern int   FS_SIZE;
extern int   BLOCK_SIZE;
extern int   retardo_operacion;
extern int   retardo_acceso_bloque;

t_log* iniciar_logger(void);
t_config* iniciar_config(char* path);
void leer_configStorage(t_config* configStorage);
void leer_configSuperBlock(t_config* configSuperBlock);
void inicializar_storage(char* path_config);

#endif

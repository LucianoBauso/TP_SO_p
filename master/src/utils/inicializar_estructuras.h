#ifndef INICIALIZAR_ESTRUCTURAS_H_
#define INICIALIZAR_ESTRUCTURAS_H_

#include <stdlib.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>

// Variables globales
extern t_log* logger;
extern t_config* config;

// Variables que se usan a lo largo de la ejecución del módulo
extern char* puerto_escucha;
extern char* algoritmo_planificacion;
extern int tiempo_aging;
extern t_log_level nivel_log;

// Prototipos de funciones
t_log* iniciar_logger(void);
t_config* iniciar_configuracion(char* ruta_config);
void leer_configuracion_master(t_config* configuracion);
void inicializar_master(void);

#endif
#ifndef MAIN_H_
#define MAIN_H_

#include <commons/log.h>
#include <commons/config.h>
#include <stdlib.h>
#include <stdio.h>
#include "argumentos.h"

// Estructuras

typedef struct {
    char* ip_master;
    char* puerto_master;
    char* log_level;
} t_config_query;

//Valores leidos del config
extern char * ip_master;
extern char* puerto_master;
extern t_log_level log_level;

// Funciones principales
t_argumentos* procesar_argumentos(int argc, char** argv);
t_config_query* cargar_configuracion(t_config* config_commons, t_log* logger);


void ejecutar_query_control(t_argumentos* args, t_config* config_qc, t_log* logger);
//void cleanup_recursos(t_argumentos* args, t_config_query* config, t_log* logger, t_config* config_commons);

// Funciones de inicialización
t_log* iniciar_logger(void);
t_config* iniciar_config(char* archivo_config);
void leer_config_qc(t_config* config_qc);

// Funciones de comunicación
//int conectar_a_master(t_config_query* config, t_log* logger);
void enviar_solicitud_query(int conexion, t_argumentos* args, t_log* logger);

#endif

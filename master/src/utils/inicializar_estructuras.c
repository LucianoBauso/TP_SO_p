#include "inicializar_estructuras.h"

t_log* logger;
t_config* config;

char* puerto_escucha;
char* algoritmo_planificacion;
int tiempo_aging;
t_log_level nivel_log;

t_log* iniciar_logger(void)
{
    t_log* nuevo_logger = log_create("master.log", "MASTER", 1, nivel_log);
    if (nuevo_logger == NULL)
    {
        perror("Error al crear el logger");
        exit(EXIT_FAILURE);
    }

    return nuevo_logger;
}

t_config* iniciar_configuracion(char* ruta_config)
{
    t_config* nueva_configuracion = config_create(ruta_config);
    if (nueva_configuracion == NULL)
    {
        perror("Error al crear la configuración");
        exit(EXIT_FAILURE);
    }

    return nueva_configuracion;
}

void leer_configuracion_master(t_config* configuracion)
{
    if (!config_has_property(configuracion, "PUERTO_ESCUCHA"))
    {
        perror("La configuración no tiene la clave PUERTO_ESCUCHA");
        config_destroy(configuracion);
        exit(EXIT_FAILURE);
    }
    puerto_escucha = config_get_string_value(configuracion, "PUERTO_ESCUCHA");
    
    algoritmo_planificacion = config_get_string_value(configuracion, "ALGORITMO_PLANIFICACION");
    tiempo_aging = config_get_int_value(configuracion, "TIEMPO_AGING");
    nivel_log = log_level_from_string(config_get_string_value(configuracion, "LOG_LEVEL"));
}

void inicializar_master(void) {
    config = iniciar_configuracion("master.config");
    leer_configuracion_master(config);
    logger = iniciar_logger();
    log_info(logger, "Logger de Master creado");
    log_info(logger, "Puerto de escucha configurado: %s", puerto_escucha);
    log_info(logger, "Algoritmo de planificación: %s", algoritmo_planificacion);
}
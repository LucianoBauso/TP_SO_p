#include "inicializar_estructuras.h"


t_log *logger;
t_config *config;

char * puerto_escucha;
char* algoritmo_planificacion;
int tiempo_aging;
t_log_level log_level;
char* path;

t_log *iniciar_logger(void)
{
    t_log *nuevo_logger = log_create("master.log", "MASTER", 1, log_level);
    if (nuevo_logger == NULL)
    {
        perror("Error al crear el logger");
        exit(EXIT_FAILURE);
    }

    return nuevo_logger;
}
/*Creacion de master.config*/
t_config *iniciar_config(char *path)
{
    t_config *nuevo_config = config_create(path);
    if (nuevo_config == NULL)
    {
        perror("Error al crear el config");
        exit(EXIT_FAILURE);
    }

    return nuevo_config;
}

void leer_configMaster(t_config *config)
{
    puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");
    if (!config_has_property(config, "PUERTO_ESCUCHA"))
    {
        perror("El config no tiene la clave PUERTO_ESCUCHA");
        config_destroy(config);
        exit(EXIT_FAILURE);
    }
    
    algoritmo_planificacion = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
    tiempo_aging = config_get_int_value(config, "TIEMPO_AGING");
    log_level = log_level_from_string(config_get_string_value(config, "LOG_LEVEL"));
}


void inicializar_master(void) {

    config = iniciar_config("master.config");
    leer_configMaster(config);
    logger = iniciar_logger();
    log_info(logger, "Creado Logger de Master");
    log_info(logger, "Leí puerto_escucha = %s", puerto_escucha);
}
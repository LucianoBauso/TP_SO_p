#include "configuracion.h"
#include <commons/config.h>
#include <commons/log.h>

t_config* iniciar_config(char* archivo_config)
{
    t_config* nuevo_config = config_create(archivo_config);
    if (!nuevo_config) {
        fprintf(stderr, "Error: No se pudo abrir el archivo de configuración: %s\n", archivo_config);
        exit(EXIT_FAILURE);
    }
    return nuevo_config;
}

void leer_config_qc (t_config* config_qc ){
    ip_master = config_get_string_value(config_qc, "IP_MASTER");
    puerto_master = config_get_string_value(config_qc, "PUERTO_MASTER");
    log_level = log_level_from_string(config_get_string_value(config_qc, "LOG_LEVEL"));
}

t_log *iniciar_logger(void)
{
    t_log *nuevo_logger = log_create("qc.log", "QUERY CONTROL", 1, log_level);
    if (nuevo_logger == NULL)
    {
        perror("Error al crear el logger");
        exit(EXIT_FAILURE);
    }

    return nuevo_logger;
}
/*
t_log* iniciar_logger(void)
{
    t_log* nuevo_logger = log_create("query_control.log", "QUERY_CONTROL", 1, LOG_LEVEL_INFO);
    if (!nuevo_logger) {
        perror("No se pudo crear el logger");
        exit(EXIT_FAILURE);
    }
    return nuevo_logger;
}
*/

/*
//Lee el archivo de configuracion basicamente
t_config_query* cargar_configuracion(t_config* config_commons, t_log* logger){
    t_config_query* config = malloc(sizeof(t_config_query));
    if (!config) {
        log_error(logger, "No se pudo asignar memoria para la configuración");
        exit(EXIT_FAILURE);
    }
    
    config->ip_master = strdup(config_get_string_value(config_commons, "IP_MASTER"));
    config->puerto_master = strdup(config_get_string_value(config_commons, "PUERTO_MASTER"));
    config->log_level = strdup(config_get_string_value(config_commons, "LOG_LEVEL"));
    
    log_info(logger, "Configuración cargada: IP_MASTER=%s PUERTO_MASTER=%s", 
             config->ip_master, config->puerto_master);
    
    return config;
}
*/


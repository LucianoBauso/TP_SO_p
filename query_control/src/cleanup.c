#include "cleanup.h"

void cleanup_recursos(t_argumentos* args, t_config_query* config, t_log* logger, t_config* config_commons)
{
    // Limpiar argumentos
    if (args) {
        free(args->archivo_config);
        free(args->archivo_query);
        free(args->prioridad);
        free(args);
    }
    
    // Limpiar configuración
    if (config) {
        free(config->ip_master);
        free(config->puerto_master);
        free(config->log_level);
        free(config);
    }
    
    // Limpiar commons
    if (config_commons) {
        config_destroy(config_commons);
    }
    
    // Limpiar logger (último)
    if (logger) {
        log_info(logger, "Finalizando Query Control - recursos liberados");
        log_destroy(logger);
    }
}
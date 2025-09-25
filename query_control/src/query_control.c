#include "main.h"
#include "utils/clientUtils.h"

void ejecutar_query_control(t_argumentos* args, t_config_query* config, t_log* logger)
{
    // 1. Conectar al Master
    int conexion = conectar_a_master(config, logger);
    
    // 2. Enviar solicitud de query
    enviar_solicitud_query(conexion, args, logger);
    
    // 3. Aquí podrías agregar lógica para esperar respuestas del Master
    // Por ahora solo cerramos la conexión
    liberar_conexion(conexion);
    log_info(logger, "Conexión con Master cerrada");
}
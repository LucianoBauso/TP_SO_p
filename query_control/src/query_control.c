#include "main.h"
#include "utils/clientUtils.h"

void ejecutar_query_control(t_argumentos* args, t_config_query* config, t_log* logger)
{
    int conexion = conectar_a_master(config, logger);
    enviar_solicitud_query(conexion, args, logger);
    liberar_conexion(conexion);
    log_info(logger, "Conexión con Master cerrada");
}

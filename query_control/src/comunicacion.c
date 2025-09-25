#include "main.h"
#include "utils/clientUtils.h"
#include <string.h>

int conectar_a_master(t_config_query* config, t_log* logger)
{
    int conexion = crear_conexion(config->ip_master, config->puerto_master, "Master");
    log_info(logger, "## Conexión al Master exitosa. IP: %s, Puerto: %s", 
             config->ip_master, config->puerto_master);
    return conexion;
}

void enviar_solicitud_query(int conexion, t_argumentos* args, t_log* logger)
{
    log_info(logger, "## Solicitud de ejecución de Query: %s, prioridad: %s", 
             args->archivo_query, args->prioridad);
             
    t_paquete* paquete = crear_paquete();
    agregar_a_paquete(paquete, args->archivo_query, strlen(args->archivo_query) + 1);
    agregar_a_paquete(paquete, args->prioridad, strlen(args->prioridad) + 1);
    
    enviar_paquete(paquete, conexion);
    eliminar_paquete(paquete);
    
    log_info(logger, "Paquete enviado exitosamente al Master");
}
#include "comunicacion.h"
#include "main.h"

int conectar_a_master(t_config* config, t_log* logger)
{
    int conexion = crear_conexion(ip_master, puerto_master, "Master");
    log_info(logger, "## Conexión al Master exitosa. IP: %s, Puerto: %s", 
             ip_master, puerto_master);
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
#include "main.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char** argv)
{
    // 1. Procesar argumentos
    t_argumentos* args = procesar_argumentos(argc, argv);
    
    // 2. Inicializar logger
    t_log* logger = iniciar_logger();
    log_info(logger, "Iniciando modulo Query Control!");
    
    // 3. Cargar configuración (UNA SOLA VEZ)
    t_config* config_commons = iniciar_config(args->archivo_config);
    t_config_query* config = cargar_configuracion(config_commons, logger);
    
    // 4. Ejecutar lógica principal
    ejecutar_query_control(args, config, logger);
    
    // 5. Limpiar recursos
    cleanup_recursos(args, config, logger, config_commons);
    
    printf("QUERY CONTROL CERRADO!!!!!\n");
    return EXIT_SUCCESS;
}

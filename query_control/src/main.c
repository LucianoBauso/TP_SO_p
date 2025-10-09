#include "main.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char** argv)
{
    t_argumentos* args = procesar_argumentos(argc, argv);
    t_log* logger = iniciar_logger();
    
    log_info(logger, "Iniciando modulo Query Control!");
    
    t_config* config_commons = iniciar_config(args->archivo_config);
    t_config_query* config = cargar_configuracion(config_commons, logger);
    
    ejecutar_query_control(args, config, logger);
    cleanup_recursos(args, config, logger, config_commons);
    printf("QUERY CONTROL CERRADO!!!!!\n");
    
    return EXIT_SUCCESS;
}

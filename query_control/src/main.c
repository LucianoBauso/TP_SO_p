#include "main.h"

//Valores leidos del config
char * ip_master;
char* puerto_master;
t_log_level log_level;

int main(int argc, char** argv)
{
    // 1. Procesar argumentos. Guarda los parametros de entrada en un struct t_argumentos. 
    t_argumentos* args = procesar_argumentos(argc, argv);

    // 2. Creo y leo el config. 
    t_config* config_qc = iniciar_config(args->archivo_config);
    leer_config_qc(config_qc);

    // 3. Creo el logger.
    t_log* logger = iniciar_logger(); 
    log_info(logger,"Creado el logger de este Query control"); 

 /*   
    // 2. Inicializar logger
    t_log* logger = iniciar_logger();
    log_info(logger, "Iniciando modulo Query Control!");
 
    // 3. Cargar configuración (UNA SOLA VEZ)
    t_config* config_commons = iniciar_config(args->archivo_config);
    t_config_query* config = cargar_configuracion(config_commons, logger);
*/    
    // 4. Ejecutar lógica principal
    ejecutar_query_control(args, config_qc, logger);
    
    // 5. Limpiar recursos
    //cleanup_recursos(args, config_qc, logger, config_commons);
    
    printf("QUERY CONTROL CERRADO!!!!!\n");
    return EXIT_SUCCESS;
}

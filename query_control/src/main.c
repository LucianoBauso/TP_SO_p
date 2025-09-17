#include "main.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "utils/clientUtils.h"      

int main(int argc, char** argv)
{
    if (argc < 4) {
        fprintf(stderr, "Uso: %s [archivo_config] [archivo_query] [prioridad]\n", argv[0]);
        return EXIT_FAILURE;
    }
    char* archivo_config = argv[1];
    char* archivo_query = argv[2];
    char* prioridadStr  = argv[3];

    t_log* logger = iniciar_logger();
    log_info(logger, "Iniciando modulo Query Control !");
    t_config* config = iniciar_config(archivo_config);

    char* ip     = config_get_string_value(config, "IP_MASTER");
    char* puerto = config_get_string_value(config, "PUERTO_MASTER");

    log_info(logger, "Configuracion: IP_MASTER=%s PUERTO_MASTER=%s", ip, puerto);

    /* ---------------- CONEXION A MASTER ---------------- */
    int conexion = crear_conexion(ip, puerto, "Master");
    log_info(logger, "## Conexión al Master exitosa. IP: %s, Puerto: %s", ip, puerto);
    log_info(logger, "## Solicitud de ejecución de Query: %s, prioridad: %s", archivo_query, prioridadStr);

    /* ---------------- ENVIO DEL PAQUETE ---------------- */
    t_paquete* p = crear_paquete();
    agregar_a_paquete(p, archivo_query, strlen(archivo_query) + 1);
    agregar_a_paquete(p, prioridadStr,  strlen(prioridadStr)  + 1);
    enviar_paquete(p, conexion);
    eliminar_paquete(p);

    terminar_programa(conexion, logger, config);

    printf("\nQUERY CONTROL CERRADO!!!!!\n");
    return 0;
}

/* ========================= HELPERS ========================= */

t_log* iniciar_logger(void)
{
    t_log* nuevo_logger = log_create("client.log", "CL_LOG", 1, LOG_LEVEL_INFO);
    if(nuevo_logger == NULL){
        perror("No se pudo crear el logger");
        exit(EXIT_FAILURE);
    }
    return nuevo_logger;
}

t_config* iniciar_config(char* archivo_config)
{
    t_config* nuevo_config = config_create(archivo_config);
    if(nuevo_config == NULL){
        perror("No se pudo crear el config");
        exit(EXIT_FAILURE);
    }
    return nuevo_config;
}

void terminar_programa(int conexion, t_log* logger, t_config* config)
{
    // Libera en orden: logger, config, conexión (usa tus utils)
    log_destroy(logger);
    config_destroy(config);
    liberar_conexion(conexion);
}

#include <utils/clientUtils.h>
#include <utils/serverUtils.h>
#include <worker.h>
#include <commons/log.h>
#include <commons/config.h>
#include <utils/protocol.h>  
#include <stdlib.h>

// Provisorio hasta usar el enum oficial de utils
#define MENSAJE 1

int main(int argc, char* argv[]) {
    int conexion_storage;
    char* ip_master;
    char* ip_storage;
    int puerto_master;
    int puerto_storage;

    t_log* logger;
    t_config* config;

    /* ---------------- LOGGING ---------------- */
    logger = log_create("worker.log", "WORKER", 1, LOG_LEVEL_INFO);
    if (logger == NULL) {
        perror("No se pudo crear el logger");
        exit(EXIT_FAILURE);
    }
    log_info(logger, "Soy Worker");

    /* ---------------- ARCHIVO DE CONFIGURACION ---------------- */
    if (argc < 2) {
        log_error(logger, "Uso: ./bin/worker <archivo_config>");
        exit(EXIT_FAILURE);
    }

    config = config_create(argv[1]);
    if (config == NULL) {
        log_error(logger, "No se pudo leer el archivo de configuración: %s", argv[1]);
        exit(EXIT_FAILURE);
    }

    ip_master = config_get_string_value(config, "IP_MASTER");
    ip_storage = config_get_string_value(config, "IP_STORAGE");
    puerto_master = config_get_int_value(config, "PUERTO_MASTER");
    puerto_storage = config_get_int_value(config, "PUERTO_STORAGE");

    log_info(logger, "Config: Master=%s:%d | Storage=%s:%d",
             ip_master, puerto_master, ip_storage, puerto_storage);

    /* ---------------- CONEXION A STORAGE ---------------- */
    char puerto_str[10];
    sprintf(puerto_str, "%d", puerto_storage);

    conexion_storage = crear_conexion(ip_storage, puerto_str, "WORKER");
    if (conexion_storage == -1) {
        log_error(logger, "No se pudo conectar a Storage en %s:%d", ip_storage, puerto_storage);
        config_destroy(config);
        log_destroy(logger);
        return EXIT_FAILURE;
    }
    log_info(logger, "Conectado a Storage en %s:%d", ip_storage, puerto_storage);

    /* ---------------- MENSAJE DE PRUEBA ---------------- */
    enviar_mensaje("hola storage, cual es el tamaño del bloque?", conexion_storage);

    // Recibir respuesta
    int cod_op = recibir_operacion(conexion_storage);
    if (cod_op == MENSAJE) {
        int size = 0;
        char* respuesta = recibir_buffer(&size, conexion_storage);
        log_info(logger, "Respuesta del Storage: %s", respuesta);
        free(respuesta);
    } else {
        log_error(logger, "Operación desconocida recibida: %d", cod_op);
    }

    liberar_conexion(conexion_storage);
    config_destroy(config);
    log_destroy(logger);

    return 0;
}

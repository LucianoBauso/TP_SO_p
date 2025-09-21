#include <commons/log.h>
#include <commons/config.h>
#include <utils/serverUtils.h>
#include <utils/clientUtils.h>
#include <utils/protocol.h>  
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <storage.h>
// Provisorio hasta que uses el enum de utils
#define MENSAJE 1

int main(int argc, char* argv[]) {
    // ----------- LOGGER -----------
    t_log* logger = log_create("storage.log", "STORAGE", 1, LOG_LEVEL_INFO);
    if (logger == NULL) {
        perror("No se pudo crear el logger");
        exit(EXIT_FAILURE);
    }

    log_info(logger, "Iniciando módulo Storage");

    // ----------- CONFIG -----------
    if (argc < 2) {
        log_error(logger, "Uso: ./bin/storage <archivo_config>");
        exit(EXIT_FAILURE);
    }

    log_info(logger, "Intentando abrir config: %s", argv[1]);
    t_config* config = config_create(argv[1]);

    if (config == NULL) {
        log_error(logger, "No se pudo leer el archivo de configuración: %s", argv[1]);
        exit(EXIT_FAILURE);
    }
    log_info(logger, "Config cargado correctamente");

    if (!config_has_property(config, "PUERTO_ESCUCHA")) {
        log_error(logger, "El config no tiene la clave PUERTO_ESCUCHA");
        config_destroy(config);
        log_destroy(logger);
        exit(EXIT_FAILURE);
    }

    char* puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");
    log_info(logger, "Leí puerto_escucha = %s", puerto_escucha);

    // ----------- SERVER -----------
    int server_fd = iniciar_servidor(puerto_escucha);
    if (server_fd == -1) {
        log_error(logger, "No se pudo iniciar el servidor en puerto %s", puerto_escucha);
        config_destroy(config);
        log_destroy(logger);
        exit(EXIT_FAILURE);
    }

    log_info(logger, "Storage escuchando en puerto %s", puerto_escucha);

    while (1) {
        int cliente_fd = esperar_cliente(server_fd);
        if (cliente_fd == -1) {
            log_error(logger, "Error al aceptar un cliente");
            continue;
        }

        log_info(logger, "Se conectó un Worker!");

        // Recibir mensaje del Worker
        int cod_op = recibir_operacion(cliente_fd);
        if (cod_op == MENSAJE) {
            int size = 0;
            char* mensaje = recibir_buffer(&size, cliente_fd);
            log_info(logger, "Pedido recibido del Worker: %s", mensaje);

            // Responder tamaño de bloque fijo
            log_info(logger, "Enviando respuesta: %d", BLOCK_SIZE);
            enviar_mensaje(BLOCK_SIZE, cliente_fd);
            log_info(logger, "Respuesta enviada correctamente");

            free(mensaje);
        } else if (cod_op == -1) {
            log_warning(logger, "El Worker cerró la conexión inesperadamente");
            close(cliente_fd);
        } else {
            log_warning(logger, "Operación desconocida recibida: %d", cod_op);
        }
    }

    // ----------- CLEANUP -----------
    config_destroy(config);
    log_destroy(logger);
    return 0;
}

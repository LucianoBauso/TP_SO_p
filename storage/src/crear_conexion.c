#include <crear_conexion.h>

void crear_server_storage (){
    int server_fd = iniciar_servidor(puerto_escucha);
    if (server_fd == -1) {
        log_error(logger, "No se pudo iniciar el servidor en puerto %s", puerto_escucha);
        config_destroy(configStorage);
        config_destroy(configSuperBlock);
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
            enviar_mensajeInt(BLOCK_SIZE, cliente_fd); 
            log_info(logger, "Respuesta enviada correctamente");

            free(mensaje);
        } else if (cod_op == -1) {
            log_warning(logger, "El Worker cerró la conexión inesperadamente");
            close(cliente_fd);
        } else {
            log_warning(logger, "Operación desconocida recibida: %d", cod_op);
        }
    }

}
   




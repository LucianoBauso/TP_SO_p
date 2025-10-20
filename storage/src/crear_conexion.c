#include "crear_conexion.h"


void crear_server_storage() {
    int server_fd = iniciar_servidor(puerto_escucha);
    if (server_fd == -1) {
        // ... (tu manejo de errores está perfecto) ...
        exit(EXIT_FAILURE);
    }
    log_info(logger, "Storage listo para recibir Workers en puerto %s", puerto_escucha);

    // El hilo main (recepcionista) se queda aceptando conexiones para siempre.
    while (1) {
        log_info(logger,"BORRAR: Antes de esperar cliente");
        int cliente_fd = esperar_cliente(server_fd);
        log_info(logger, "BORRAR: despues de esperar cleinte");
        if (cliente_fd == -1) {
            log_error(logger, "Error al aceptar un cliente, reintentando...");
            continue;
        }

        log_info(logger, "¡Se conectó un Worker! Creando un hilo para atenderlo...");

        // 1. Reservamos memoria para pasar el socket de forma segura al hilo.
        int* arg_cliente_fd = malloc(sizeof(int));
        *arg_cliente_fd = cliente_fd;

        // 2. Creamos el hilo que ejecutará la función atender_worker.
        pthread_t hilo_worker;
        if (pthread_create(&hilo_worker, NULL, atender_worker, arg_cliente_fd) != 0) {
            log_error(logger, "Error al crear el hilo para el nuevo Worker");
            free(arg_cliente_fd); // Si falla, liberamos la memoria nosotros.
            continue;
        }

        // 3. Desacoplamos el hilo para no tener que esperarlo (con pthread_join).
        // Esto permite que sus recursos se liberen automáticamente cuando termine.
        pthread_detach(hilo_worker);
    }
}
   

// Esta es la función que cada hilo ejecutará.
// Contiene la lógica para comunicarse con un único Worker.
void* atender_worker(void* arg) {
    // 1. Recibimos el file descriptor del socket del cliente.
    int cliente_fd = *(int*)arg;
    free(arg); // 2. Liberamos la memoria usada para pasar el argumento. ¡Crucial!

    log_info(logger, "[Hilo %lu] Atendiendo nuevo Worker en socket %d", pthread_self(), cliente_fd);

    // 3. La misma lógica de antes, pero ahora dentro de un hilo.
    int cod_op = recibir_operacion(cliente_fd);
    if (cod_op == MENSAJE) {
        int size = 0;
        char* mensaje = recibir_buffer(&size, cliente_fd);
        log_info(logger, "[Hilo %lu] Pedido recibido del Worker: %s", pthread_self(), mensaje);

        // Responder tamaño de bloque fijo
        log_info(logger, "[Hilo %lu] Enviando respuesta: %d", pthread_self(), BLOCK_SIZE);
        enviar_mensajeInt(BLOCK_SIZE, cliente_fd);
        log_info(logger, "[Hilo %lu] Respuesta enviada correctamente", pthread_self());

        free(mensaje);
    } else if (cod_op == -1) {
        log_warning(logger, "[Hilo %lu] El Worker en socket %d cerró la conexión inesperadamente", pthread_self(), cliente_fd);
    } else {
        log_warning(logger, "[Hilo %lu] Operación desconocida recibida: %d", pthread_self(), cod_op);
    }

    // 4. El hilo terminó su trabajo, cerramos la conexión de este worker.
    log_info(logger, "[Hilo %lu] Tarea finalizada. Cerrando conexión del socket %d.", pthread_self(), cliente_fd);
    close(cliente_fd);
    return NULL;
}
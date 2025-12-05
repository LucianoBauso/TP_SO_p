#include "crear_conexion.h"
#include <pthread.h>
#include "protocol.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

static pthread_mutex_t mutex_workers = PTHREAD_MUTEX_INITIALIZER;
static int cantidad_workers = 0;

// Helper para deserializar string del buffer (formato: [int len][bytes])
static char* deserializar_string(void* buffer, int* offset) {
    int len = 0;
    memcpy(&len, buffer + *offset, sizeof(int));
    *offset += sizeof(int);

    char* str = malloc(len + 1);
    memcpy(str, buffer + *offset, len);
    str[len] = '\0';
    *offset += len;
    return str;
}

void crear_server_storage(void) {
    int server_fd = iniciar_servidor(puerto_escucha);
    if (server_fd == -1) {
        perror("No se pudo iniciar el servidor de Storage");
        exit(EXIT_FAILURE);
    }
    log_info(logger, "Storage listo para recibir Workers en puerto %s", puerto_escucha);

    while (1) {
        int cliente_fd = esperar_cliente(server_fd);
        if (cliente_fd == -1) {
            log_error(logger, "Error en aceptar cliente en Storage");
            continue;
        }

        // 1. Reservar memoria para pasar el socket de forma segura al hilo.
        int* arg_cliente_fd = malloc(sizeof(int));
        if (arg_cliente_fd == NULL) {
            log_error(logger, "malloc falló al reservar arg_cliente_fd");
            close(cliente_fd);
            continue;
        }
        *arg_cliente_fd = cliente_fd;

        // 2. Crear el hilo que ejecutará la función atender_worker.
        pthread_t hilo_worker;
        if (pthread_create(&hilo_worker, NULL, atender_worker, arg_cliente_fd) != 0) {
            log_error(logger, "Error al crear el hilo para el nuevo Worker");
            free(arg_cliente_fd); // Si falla, liberamos la memoria nosotros.
            close(cliente_fd);
            continue;
        }

        // 3. Detach: no vamos a hacer join, el hilo se limpia solo al terminar.
        pthread_detach(hilo_worker);
    }
}


void* atender_worker(void* arg) {
    int cliente_fd = *(int*)arg;
    free(arg);

    pthread_mutex_lock(&mutex_workers);
    cantidad_workers++;
    int worker_id = cantidad_workers;
    log_info(logger, "Se conecta un Worker (id interno %d). Cantidad de Workers conectados: %d",
             worker_id, cantidad_workers);
    pthread_mutex_unlock(&mutex_workers);

    bool seguir = true;

    while (seguir) {

        int cod_op = recibir_operacion(cliente_fd);
        if (cod_op == -1) {
            log_warning(logger, "[Worker %d] Cerró la conexión", worker_id);
            break;
        }

        switch (cod_op) {

        case MENSAJE:
        case OP_STORAGE_HANDSHAKE: {
            int size = 0;
            void* buffer = recibir_buffer(&size, cliente_fd);
            free(buffer);

            enviar_mensajeInt(BLOCK_SIZE, cliente_fd);
            log_info(logger, "[Worker %d] Handshake atendido, se envió BLOCK_SIZE=%d",
                     worker_id, BLOCK_SIZE);
            break;
        }

        case OP_STORAGE_CREATE: {
            int size = 0;
            void* buffer = recibir_buffer(&size, cliente_fd);
            if (!buffer) {
                log_error(logger, "[Worker %d] Error recibiendo OP_STORAGE_CREATE", worker_id);
                break;
            }

            int offset = 0;
            int query_id;
            memcpy(&query_id, buffer + offset, sizeof(int));
            offset += sizeof(int);

            char* file = deserializar_string(buffer, &offset);
            char* tag  = deserializar_string(buffer, &offset);

            fs_result_t r = fs_crear_file_tag(file, tag, query_id);
            int resultado = (int)r;
            send(cliente_fd, &resultado, sizeof(int), 0);

            free(file);
            free(tag);
            free(buffer);
            break;
        }

        case OP_STORAGE_TRUNCATE: {
            int size = 0;
            void* buffer = recibir_buffer(&size, cliente_fd);
            if (!buffer) {
                log_error(logger, "[Worker %d] Error recibiendo OP_STORAGE_TRUNCATE", worker_id);
                break;
            }

            int offset = 0;
            int query_id;
            memcpy(&query_id, buffer + offset, sizeof(int));
            offset += sizeof(int);

            char* file = deserializar_string(buffer, &offset);
            char* tag  = deserializar_string(buffer, &offset);

            uint32_t nuevo_tamanio;
            memcpy(&nuevo_tamanio, buffer + offset, sizeof(uint32_t));

            fs_result_t r = fs_truncar(file, tag, nuevo_tamanio, query_id);
            int resultado = (int)r;
            send(cliente_fd, &resultado, sizeof(int), 0);

            free(file);
            free(tag);
            free(buffer);
            break;
        }

        case OP_STORAGE_READ_BLOCK: {
            int size = 0;
            void* buffer = recibir_buffer(&size, cliente_fd);
            if (!buffer) {
                log_error(logger, "[Worker %d] Error recibiendo OP_STORAGE_READ_BLOCK", worker_id);
                break;
            }

            int offset = 0;
            int query_id;
            memcpy(&query_id, buffer + offset, sizeof(int));
            offset += sizeof(int);

            char* file = deserializar_string(buffer, &offset);
            char* tag  = deserializar_string(buffer, &offset);

            uint32_t nro_bloque_logico;
            memcpy(&nro_bloque_logico, buffer + offset, sizeof(uint32_t));

            free(buffer);

            void* bloque = malloc(BLOCK_SIZE);

            fs_result_t r =
                fs_leer_bloque(file, tag, nro_bloque_logico, bloque, query_id);

            int resultado = (int)r;
            send(cliente_fd, &resultado, sizeof(int), 0);
            if (r == FS_OK) {
                send(cliente_fd, bloque, BLOCK_SIZE, 0);
            }

            free(bloque);
            free(file);
            free(tag);
            break;
        }

        case OP_STORAGE_WRITE_BLOCK: {
            int size = 0;
            void* buffer = recibir_buffer(&size, cliente_fd);
            if (!buffer) {
                log_error(logger, "[Worker %d] Error recibiendo OP_STORAGE_WRITE_BLOCK", worker_id);
                break;
            }

            int offset = 0;
            int query_id;
            memcpy(&query_id, buffer + offset, sizeof(int));
            offset += sizeof(int);

            char* file = deserializar_string(buffer, &offset);
            char* tag  = deserializar_string(buffer, &offset);

            uint32_t nro_bloque_logico;
            memcpy(&nro_bloque_logico, buffer + offset, sizeof(uint32_t));
            offset += sizeof(uint32_t);

            int bytes_restantes = size - offset;
            if (bytes_restantes != BLOCK_SIZE) {
                log_warning(logger, "[Worker %d] WRITE_BLOCK tamaño inesperado (%d, esperaba %d)",
                            worker_id, bytes_restantes, BLOCK_SIZE);
            }

            fs_result_t r = fs_escribir_bloque(file, tag, nro_bloque_logico,
                                               buffer + offset, query_id);

            int resultado = (int)r;
            send(cliente_fd, &resultado, sizeof(int), 0);

            free(file);
            free(tag);
            free(buffer);
            break;
        }

        default:
            log_warning(logger, "[Worker %d] Operación desconocida recibida: %d",
                        worker_id, cod_op);
            seguir = false;
            break;
        }
    }

    // cierre correcto de hilo
    close(cliente_fd);

    pthread_mutex_lock(&mutex_workers);
    cantidad_workers--;
    pthread_mutex_unlock(&mutex_workers);

    return NULL;
}


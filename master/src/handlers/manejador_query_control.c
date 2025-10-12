#include "manejador_query_control.h"
#include "../state/estado_master.h"
#include "../core/master.h"
#include "utils/serverUtils.h"
#include <commons/collections/list.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

static int obtener_nivel_multiprocesamiento_actual(void) {
    return 0; // TODO: reemplazar cuando tengamos la lista de workers conectados
}

void registrar_query_control(int socket_query_control) {
    t_conexion_query_control* conexion = malloc(sizeof(t_conexion_query_control));
    conexion->socket_query_control = socket_query_control;
    conexion->queries = list_create(); // por ahora vacío

    pthread_mutex_lock(&MUTEX_QUERY_CONTROL);
    list_add(CONEXIONES_QUERY_CONTROL, conexion);
    pthread_mutex_unlock(&MUTEX_QUERY_CONTROL);

    if (logger) {
        log_info(logger, "## Conexión de Query Control registrada (socket=%d)", socket_query_control);
    }
}

void remover_query_control_y_cancelar(int socket_query_control) {
    pthread_mutex_lock(&MUTEX_QUERY_CONTROL);
    for (int i = 0; i < list_size(CONEXIONES_QUERY_CONTROL); i++) {
        t_conexion_query_control* conexion = list_get(CONEXIONES_QUERY_CONTROL, i);
        if (conexion->socket_query_control == socket_query_control) {
            // En pasos siguientes: cancelar sus queries (READY/EXEC) y loguear según enunciado
            list_remove(CONEXIONES_QUERY_CONTROL, i);
            if (conexion->queries) list_destroy(conexion->queries);
            free(conexion);
            break;
        }
    }
    pthread_mutex_unlock(&MUTEX_QUERY_CONTROL);

    if (logger) {
        log_info(logger, "## Se desconecta un Query Control (socket=%d)", socket_query_control);
    }
    close(socket_query_control);
}

void manejar_query_control(int socket_query_control) {
    registrar_query_control(socket_query_control);

    while (1) {
        int operacion = recibir_operacion(socket_query_control);
        if (operacion <= 0) { // error o desconexión
            remover_query_control_y_cancelar(socket_query_control);
            break;
        }

        if (operacion == PAQUETE) {
            t_list* elementos = recibir_paquete(socket_query_control);
            if (!elementos || list_size(elementos) < 2) {
                if (logger) log_warning(logger, "Query Control (socket=%d): paquete incompleto", socket_query_control);
                if (elementos) list_destroy(elementos);
                continue;
            }

            char* path_query = list_get(elementos, 0);
            char* prioridad_str = list_get(elementos, 1);
            int prioridad = atoi(prioridad_str);

            t_query* query = crear_query(socket_query_control, path_query, prioridad);

            asociar_query_a_conexion(socket_query_control, query);
            encolar_en_ready(query);

            int nivel_multiprocesamiento = 0; // TODO: reemplazar por cantidad de workers conectados

            log_info(logger,
                "## Se conecta un Query Control para ejecutar la Query %s con prioridad %d - Id asignado: %d. Nivel multiprocesamiento %d",
                path_query, prioridad, query->query_id, nivel_multiprocesamiento
            );

            //log_debug(logger, "READY <- %s (id=%d). Tamaño READY=%d", path_query, query->query_id, cantidad_en_ready());
            log_info(logger, "READY: %s (id=%d). Tamaño READY=%d", path_query, query->query_id, cantidad_en_ready());

            enviar_mensaje("ACK_SUBMIT", socket_query_control);

            void _liberar(void* elemento) { free(elemento); }
            list_destroy_and_destroy_elements(elementos, _liberar);

            // NO cerramos el socket, seguimos escuchando
        }
        else if (operacion == MENSAJE) {
            recibir_mensaje(socket_query_control);
        }
        else {
            if (logger) log_warning(logger, "Query Control (socket=%d): operación desconocida=%d", socket_query_control, operacion);
            continue;
        }
    }
}

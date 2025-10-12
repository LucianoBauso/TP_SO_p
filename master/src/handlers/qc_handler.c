#include "qc_handler.h"
#include "../state/master_state.h"
#include "../core/master.h"
#include "utils/serverUtils.h"
#include <commons/collections/list.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

static int nivel_multiprocesamiento_actual(void) {
    return 0; // TODO: reemplazar cuando tengamos la lista de workers conectados
}

void registrar_qc(int qc_socket) {
    t_qc_conn* qc = malloc(sizeof(t_qc_conn));
    qc->qc_socket = qc_socket;
    qc->queries = list_create(); // por ahora vacío

    pthread_mutex_lock(&M_QC);
    list_add(QC_CONNS, qc);
    pthread_mutex_unlock(&M_QC);

    if (logger) {
        log_info(logger, "## Conexión de Query Control registrada (socket=%d)", qc_socket);
    }
}

void remover_qc_y_cancelar(int qc_socket) {
    pthread_mutex_lock(&M_QC);
    for (int i = 0; i < list_size(QC_CONNS); i++) {
        t_qc_conn* it = list_get(QC_CONNS, i);
        if (it->qc_socket == qc_socket) {
            // En pasos siguientes: cancelar sus queries (READY/EXEC) y loguear según enunciado
            list_remove(QC_CONNS, i);
            if (it->queries) list_destroy(it->queries);
            free(it);
            break;
        }
    }
    pthread_mutex_unlock(&M_QC);

    if (logger) {
        log_info(logger, "## Se desconecta un Query Control (socket=%d)", qc_socket);
    }
    close(qc_socket);
}

void manejar_query_control(int qc_socket) {
    registrar_qc(qc_socket);

    while (1) {
        int op = recibir_operacion(qc_socket);
        if (op <= 0) { // error o desconexión
            remover_qc_y_cancelar(qc_socket);
            break;
        }

        if (op == PAQUETE) {
            t_list* items = recibir_paquete(qc_socket);
            if (!items || list_size(items) < 2) {
                if (logger) log_warning(logger, "QC(socket=%d): paquete incompleto", qc_socket);
                if (items) list_destroy(items);
                continue;
            }

            char* path_query = list_get(items, 0);
            char* prio_str   = list_get(items, 1);
            int   prioridad  = atoi(prio_str);

            t_query* q = query_new(qc_socket, path_query, prioridad);

            qc_attach_query(qc_socket, q);    //Asociarla al QC
            ready_enqueue(q);    //Encolar a ready

            int nivel_mp = 0; // TODO: reemplazar por cantidad de workers conectados

            log_info(logger,
                "## Se conecta un Query Control para ejecutar la Query %s con prioridad %d - Id asignado: %d. Nivel multiprocesamiento %d",
                path_query, prioridad, q->query_id, nivel_mp
            );

            //log_info(logger, "READY <- %s (id=%d). Tamaño READY=%d", path_query, q->query_id, ready_count());
            log_debug(logger, "READY <- %s (id=%d). Tamaño READY=%d", path_query, q->query_id, ready_count());
            
            enviar_mensaje("ACK_SUBMIT", qc_socket);

            void _fre(void* x){ free(x); }
            list_destroy_and_destroy_elements(items, _fre);

            // NO cerramos el socket, seguimos escuchando
        }
        else if (op == MENSAJE) {
            recibir_mensaje(qc_socket);
        }
        else {
            if (logger) log_warning(logger, "QC(socket=%d): op desconocida=%d", qc_socket, op);
            continue;
        }
    }
}

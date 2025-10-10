#include "master.h"

//t_log* logger;

//define qué tipo de conexión recibe - si worker o QC
void* manejar_cliente(void* arg) {
    int sock = *(int*)arg;
    free(arg);  // liberar memoria del socket

    int tipo = -1; // 0 = QC, 1 = Worker
    recv(sock, &tipo, sizeof(tipo), 0);

    if (tipo == 0) {
        log_info(logger,"Se conecto un Query Control");
        manejar_query_control(sock);
    } else if (tipo == 1) {
        log_info(logger,"Se conecto un Worker");
        manejar_worker(sock);
    } else {
        log_warning(logger, "Cliente desconocido, cerrando socket");
        close(sock);
    }

    return NULL;
}





// Esta función se encarga de un Query Control específico
void manejar_query_control(int sock_qc) {
    int next_query_id = 0;
    int nivel_mp = 0;

    int op = recibir_operacion(sock_qc);

    if (op == PAQUETE) {
        t_list* items = recibir_paquete(sock_qc);
        char* path_query = list_get(items, 0);
        char* prio_str   = list_get(items, 1);
        int prioridad    = atoi(prio_str);
        int query_id     = next_query_id++;

        log_info(logger, "## Se conecta un Query Control para ejecutar la Query %s con prioridad %d - Id asignado: %d. Nivel multiprocesamiento %d",
                 path_query, prioridad, query_id, nivel_mp);

        enviar_mensaje("ACK_SUBMIT", sock_qc);

        void _fre(void* x){ free(x); }
        list_destroy_and_destroy_elements(items, _fre);
    } else if (op == MENSAJE) {
        recibir_mensaje(sock_qc);
    }

    close(sock_qc);
}

// Esta función se encarga de un Worker específico
void manejar_worker(int sock_worker) {
    int worker_id;
    recv(sock_worker, &worker_id, sizeof(int), 0); // ejemplo de handshake

    // Aquí actualizarías estructuras de Workers
    log_info(logger, "## Se conecta el Worker %d - Cantidad total de Workers: <actualizar>", worker_id);

    // Ejemplo de ciclo de comunicación con el Worker
    // recibir tareas, enviar Query, etc.

    close(sock_worker);
}





int ejecutar_master(void) {
    //logger = log_create("master.log", "MASTER", 1, LOG_LEVEL_INFO);

    int sock_srv = iniciar_servidor(puerto_escucha);                  // escucha en PUERTO "9001"
    if (sock_srv == -1) {
        log_error(logger, "No se pudo iniciar el servidor en puerto %s", puerto_escucha);
        config_destroy(config);
        log_destroy(logger);
        exit(EXIT_FAILURE);
    }
    log_info(logger, "Master escuchando en %s ...", puerto_escucha);

    //EStos por ahí haya que borrarlos
    int next_query_id = 0;
    int nivel_mp = 0;

    //aca arranca while(t) ?
    while (1) {
        int* client_sock_ptr = malloc(sizeof(int));
        *client_sock_ptr = esperar_cliente(sock_srv);
        if (*client_sock_ptr == -1) {
            free(client_sock_ptr);
            continue;
        }

        pthread_t hilo;
        pthread_create(&hilo, NULL, manejar_cliente, client_sock_ptr);
        pthread_detach(hilo);
    }

}
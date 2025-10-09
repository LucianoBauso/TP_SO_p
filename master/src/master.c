#include "master.h"

//t_log* logger;
void* manejar_cliente(void* arg) {
    int sock = *(int*)arg;
    int tipo; // 0 = QC, 1 = worker,  otro es desconocido y cierra el socket. ---------------------------------------------
    recv(sock, tipo, sizeof(tipo), 0);

    if (tipo == 0) {
        manejar_query_control(sock); // tu función que atiende QC
    } else if (tipo == 1) {
        manejar_worker(sock);       // tu función que atiende Worker
    } else {
        close(sock); // cliente desconocido
    }
    return NULL;
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

    int next_query_id = 0;
    int nivel_mp = 0;
//aca arranca while(t) ?
while(1){


    int sock_qc = esperar_cliente(sock_srv);
    log_info(logger,"Se conecto un cliente");
    int op = recibir_operacion(sock_qc);

    if (op == PAQUETE) {
        t_list* items = recibir_paquete(sock_qc);
        char* path_query = list_get(items, 0);
        char* prio_str   = list_get(items, 1);
        int prioridad    = atoi(prio_str);
        int query_id     = next_query_id++;

        log_info(logger, "===== NUEVA CONEXION QUERY CONTROL =====");
        log_info(logger, "Query Path: %s", path_query);
        log_info(logger, "Prioridad: %d", prioridad);
        log_info(logger, "Query ID asignado: %d", query_id);
        log_info(logger, "Nivel multiprocesamiento: %d", nivel_mp);
        log_info(logger, "========================================");

        enviar_mensaje("ACK_SUBMIT", sock_qc);

        void _fre(void* x){ free(x); }
        list_destroy_and_destroy_elements(items, _fre);
    } else if (op == MENSAJE) {
        recibir_mensaje(sock_qc);
    }

    close(sock_qc);
}
    close(sock_srv);
    //log_destroy(logger);
    return 0;
}

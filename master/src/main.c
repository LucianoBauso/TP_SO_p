#include <stdlib.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include "utils/serverUtils.h"
#include "utils/clientUtils.h"

t_log* logger;

int main(void) {
    logger = log_create("master.log", "MASTER", 1, LOG_LEVEL_INFO);

    int sock_srv = iniciar_servidor("9001");                  // escucha en PUERTO "9001"
    log_info(logger, "Master escuchando en %s ...", "9001");

    int next_query_id = 0;
    int nivel_mp = 0;

    int sock_qc = esperar_cliente(sock_srv);
    int op = recibir_operacion(sock_qc);

    if (op == PAQUETE) {
        t_list* items = recibir_paquete(sock_qc);
        char* path_query = list_get(items, 0);
        char* prio_str   = list_get(items, 1);
        int prioridad    = atoi(prio_str);
        int query_id     = next_query_id++;

        log_info(logger,
          "## Se conecta un Query Control para ejecutar la Query %s con prioridad %d - Id asignado: %d. Nivel multiprocesamiento %d",
          path_query, prioridad, query_id, nivel_mp);

        enviar_mensaje("ACK_SUBMIT", sock_qc);

        void _fre(void* x){ free(x); }
        list_destroy_and_destroy_elements(items, _fre);
    } else if (op == MENSAJE) {
        recibir_mensaje(sock_qc);
    }

    close(sock_qc);
    close(sock_srv);
    log_destroy(logger);
    return 0;
}

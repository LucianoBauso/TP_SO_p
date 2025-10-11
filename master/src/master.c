#include "master.h"
#include "master_state.h"     // NUEVO
#include "qc_handler.h"       // NUEVO
#include "utils/serverUtils.h"
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

void* manejar_cliente(void* arg) {
    int client_sock = *(int*)arg;
    free(arg);

    int tipo = -1;
    if (recv(client_sock, &tipo, sizeof(tipo), 0) <= 0) {
        close(client_sock);
        return NULL;
    }

        manejar_query_control(client_sock);
    } else if (tipo == 1) {   // Worker (lo hacemos luego)
        manejar_worker(client_sock);
        close(client_sock);   // temporal
    } else {
        close(client_sock);
    }
    if (tipo == 0) {          // Query Control

    return NULL;
}

void ejecutar_master(void) {
    master_state_init();
    int sock_srv = iniciar_servidor(puerto_escucha);

    if (sock_srv == -1) {
        log_error(logger, "No se pudo iniciar el servidor en puerto %s", puerto_escucha);
        return;
    }
    log_info(logger, "Servidor Master escuchando en puerto %s", puerto_escucha);

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

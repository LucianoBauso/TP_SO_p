#include "master.h"
#include "../state/estado_master.h"
#include "../handlers/manejador_query_control.h"
#include "../handlers/manejador_worker.h"
#include "utils/serverUtils.h"
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

void* manejar_cliente(void* argumento) {
    int socket_cliente = *(int*)argumento;
    free(argumento);

    int tipo_conexion = -1;
    if (recv(socket_cliente, &tipo_conexion, sizeof(tipo_conexion), 0) <= 0) {
        close(socket_cliente);
        return NULL;
    }

    if (tipo_conexion == 0) {         
        manejar_query_control(socket_cliente);
    } else if (tipo_conexion == 1) {   
        manejar_worker(socket_cliente);
    } else {
        close(socket_cliente);
    }

    return NULL;
}

void ejecutar_master(void) {
    inicializar_estado_master();
    int socket_servidor = iniciar_servidor(puerto_escucha);

    if (socket_servidor == -1) {
        log_error(logger, "No se pudo iniciar el servidor en puerto %s", puerto_escucha);
        return;
    }
    log_info(logger, "Servidor Master escuchando en puerto %s", puerto_escucha);

    while (1) {
        int* socket_cliente_ptr = malloc(sizeof(int));
        *socket_cliente_ptr = esperar_cliente(socket_servidor);
        if (*socket_cliente_ptr == -1) {
            free(socket_cliente_ptr);
            continue;
        }

        pthread_t hilo;
        pthread_create(&hilo, NULL, manejar_cliente, socket_cliente_ptr);
        pthread_detach(hilo); // No esperamos a que termine el hilo, lo dejamos correr y se limpia solo
    }
}

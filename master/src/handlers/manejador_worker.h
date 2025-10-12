#ifndef MANEJADOR_WORKER_H
#define MANEJADOR_WORKER_H

#include "../state/estado_master.h"

typedef struct {
    int socket_worker;
    int ocupado; // 0 libre, 1 ocupado
} t_worker;

// Inicialización y registro de Worker
void manejar_worker(int socket_worker);
void registrar_worker(int socket_worker);
void remover_worker(int socket_worker);
void planificador_intentar_despachar(void);    // Planificación FIFO: intenta despachar una Query si hay Worker libre

#endif

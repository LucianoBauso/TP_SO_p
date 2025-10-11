#ifndef WORKER_HANDLER_H
#define WORKER_HANDLER_H

#include "master_state.h"

typedef struct {
    int worker_sock;
    int ocupado; // 0 libre, 1 ocupado
} t_worker;

// Inicialización y registro de Worker
void manejar_worker(int worker_sock);
void registrar_worker(int worker_sock);
void remover_worker(int worker_sock);
void planificador_intentar_despachar(void);    //Planificación FIFO: intenta despachar una Query si hay Worker libre

#endif

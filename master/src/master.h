#ifndef MASTER_H
#define MASTER_H

#include <stdlib.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include "utils/serverUtils.h"
#include "utils/clientUtils.h"
#include "inicializar_estructuras.h"
#include <pthread.h>

//extern t_log* logger;

/**
 * @brief Ejecuta la lógica principal del master
 * @return Código de salida del programa
 */
void ejecutar_master(void);
void* manejar_cliente(void* arg);
void manejar_query_control(int sock_qc);
void manejar_worker(int sock_worker);

#endif // MASTER_H

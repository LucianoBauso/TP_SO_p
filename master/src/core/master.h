#ifndef MASTER_H
#define MASTER_H

#include <stdlib.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include "utils/serverUtils.h"
#include "utils/clientUtils.h"
#include "../utils/inicializar_estructuras.h"
#include <pthread.h>

/**
 * @brief Ejecuta la lógica principal del master
 */
void ejecutar_master(void);
void* manejar_cliente(void* argumento);

#endif // MASTER_H

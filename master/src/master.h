#ifndef MASTER_H
#define MASTER_H

#include <stdlib.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include "utils/serverUtils.h"
#include "utils/clientUtils.h"

extern t_log* logger;

/**
 * @brief Ejecuta la lógica principal del master
 * @return Código de salida del programa
 */
int ejecutar_master(void);

#endif // MASTER_H

#ifndef CREAR_CONEXION_H_
#define CREAR_CONEXION_H_

#include <utils/serverUtils.h>
#include <utils/clientUtils.h>
#include "inicializar_estructuras.h"
#include "filesystem.h"
#include <pthread.h>

void crear_server_storage(void);
void* atender_worker(void* arg);

#endif
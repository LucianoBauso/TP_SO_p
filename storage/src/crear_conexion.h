#ifndef CONEXION_H_
#define CONEXION_H_

#include <utils/serverUtils.h>
#include <utils/clientUtils.h>
#include <inicializar_estructuras.h>
#include <pthread.h>

extern int BLOCK_SIZE;
extern char* puerto_escucha;

void crear_server_storage (void);
void* atender_worker(void* arg) ;

#endif
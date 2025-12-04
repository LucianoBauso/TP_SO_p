#ifndef CONEXION_H_
#define CONEXION_H_


#include <utils/serverUtils.h>
#include <utils/clientUtils.h>
#include <inicializar_estructuras.h>


void crear_client_worker_storage(void);

int conectar_a_master();
int conectar_a_storage(); 

void enviar_ID_WORKER_a_master(int conexion_master,int id_worker);
void enviar_pedido_BLOCKSIZE_a_storage(int conexion_storage);

#endif
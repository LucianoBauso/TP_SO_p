#ifndef SERVER_UTILS_H_
#define SERVER_UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <string.h>
#include <assert.h>

#include "protocol.h"

// -------------------- FUNCIONES --------------------

// Inicia un servidor en el puerto especificado (ej: "4444")
// Devuelve el file descriptor del socket o -1 si falla
int iniciar_servidor(char* puerto);

// Espera a que un cliente se conecte
int esperar_cliente(int socket_servidor);

// Recibe el código de operación enviado por el cliente
int recibir_operacion(int socket_cliente);

// Recibe un buffer del cliente
void* recibir_buffer(int* size, int socket_cliente);

// Recibe y loguea un mensaje (usa commons log)
void recibir_mensaje(int socket_cliente);

// Recibe un paquete (lista de valores)
t_list* recibir_paquete(int socket_cliente);

#endif /* SERVER_UTILS_H_ */

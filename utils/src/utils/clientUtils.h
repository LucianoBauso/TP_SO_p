#ifndef CLIENT_UTILS_H_
#define CLIENT_UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>

#include <commons/log.h>
#include "protocol.h"   // <-- enum compartido

typedef struct {
    int size;
    void* stream;
} t_buffer;

typedef struct {
    op_code codigo_operacion;
    t_buffer* buffer;
} t_paquete;

// Funciones principales
int crear_conexion(char* ip, char* puerto, char* modulo);
void enviar_mensaje(char* mensaje, int socket_cliente);
void enviar_mensajeInt(int mensaje, int socket_cliente);
t_paquete* crear_paquete(void);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void eliminar_paquete(t_paquete* paquete);
void liberar_conexion(int socket_cliente);

#endif /* CLIENT_UTILS_H_ */

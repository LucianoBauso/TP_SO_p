#include "serverUtils.h"

int iniciar_servidor(char* puerto) {
    struct addrinfo hints, *servinfo, *p;
    int socket_servidor;
    int yes = 1;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int rv;
    if ((rv = getaddrinfo(NULL, puerto, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return -1;
    }

    // Intentar bind en alguna dirección válida
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("socket");
            continue;
        }

        if (setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            close(socket_servidor);
            continue;
        }

        if (bind(socket_servidor, p->ai_addr, p->ai_addrlen) == -1) {
            perror("bind");
            close(socket_servidor);
            continue;
        }

        break; // bind OK
    }

    freeaddrinfo(servinfo);

    if (p == NULL) {
        fprintf(stderr, "Error: no pude hacer bind al puerto %s\n", puerto);
        return -1;
    }

    if (listen(socket_servidor, SOMAXCONN) == -1) {
        perror("listen");
        return -1;
    }

    return socket_servidor;
}

int esperar_cliente(int socket_servidor) {
    int socket_cliente = accept(socket_servidor, NULL, NULL);
    if (socket_cliente == -1) {
        perror("accept");
    }
    return socket_cliente;
}

int recibir_operacion(int socket_cliente) {
    int cod_op;
    if (recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
        return cod_op;
    else {
        close(socket_cliente);
        return -1;
    }
}

void* recibir_buffer(int* size, int socket_cliente) {
    void* buffer = NULL;
    if (recv(socket_cliente, size, sizeof(int), MSG_WAITALL) <= 0)
        return NULL;

    buffer = malloc(*size);
    if (recv(socket_cliente, buffer, *size, MSG_WAITALL) <= 0) {
        free(buffer);
        return NULL;
    }

    return buffer;
}

void recibir_mensaje(int socket_cliente) {
    int size;
    char* buffer = recibir_buffer(&size, socket_cliente);
    if (buffer != NULL) {
        printf("Me llegó el mensaje: %s\n", buffer);
        free(buffer);
    }
}

t_list* recibir_paquete(int socket_cliente) {
    int size;
    int desplazamiento = 0;
    void* buffer;
    t_list* valores = list_create();
    int tamanio;

    buffer = recibir_buffer(&size, socket_cliente);
    if (buffer == NULL) return valores;

    while (desplazamiento < size) {
        memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
        desplazamiento += sizeof(int);
        char* valor = malloc(tamanio);
        memcpy(valor, buffer + desplazamiento, tamanio);
        desplazamiento += tamanio;
        list_add(valores, valor);
    }
    free(buffer);
    return valores;
}

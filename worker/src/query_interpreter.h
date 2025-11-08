#ifndef QUERY_INTERPRETER_H_
#define QUERY_INTERPRETER_H_

#include <stdbool.h>
#include <commons/log.h>
#include "inicializar_estructuras.h"

typedef enum {
    IN_CREATE,
    IN_TRUNCATE,
    IN_WRITE,
    IN_READ,
    IN_TAG,
    IN_COMMIT,
    IN_FLUSH,
    IN_DELETE,
    IN_END,
    IN_INVALIDA // En caso d error
} t_instruccion_worker;

extern bool worker_activo;

t_instruccion_worker parsear_instruccion(char* linea, char** arg1, char** arg2, char** arg3, char** arg4);

void activar_QI(int conexion_master, int conexion_storage);
void execute_create(char* file_name, char* tag, int conexion_storage);
void execute_truncate(char* file_name, char* tag, int size, int conexion_storage);
void execute_write(char* file_name, char* tag, int base_address, char* content, int conexion_storage);
void execute_read(char* file_name, char* tag, int conexion_storage);
char* recibir_operacion_master(); // TODO correjir

#endif
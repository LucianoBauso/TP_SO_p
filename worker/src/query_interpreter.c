#include "query_interpreter.h"
#include <string.h>
#include <stdlib.h>
#include <utils/protocol.h>
#include <utils/clientUtils.h>
#include <utils/serverUtils.h>

bool worker_activo;

t_instruccion_worker parsear_instruccion(char* linea, char** arg1, char** arg2, char** arg3, char** arg4) {
    char* instruccion = strtok(linea, " ");
    if (strcmp(instruccion, "CREATE") == 0) {
        *arg1 = strtok(NULL, ":");
        *arg2 = strtok(NULL, "");
        return IN_CREATE;
    }
    if (strcmp(instruccion, "TRUNCATE") == 0) {
        *arg1 = strtok(NULL, ":");
        *arg2 = strtok(NULL, " ");
        *arg3 = strtok(NULL, "");
        return IN_TRUNCATE;
    }
    if (strcmp(instruccion, "WRITE") == 0) {
        *arg1 = strtok(NULL, ":");
        *arg2 = strtok(NULL, " ");
        *arg3 = strtok(NULL, " ");
        *arg4 = strtok(NULL, "");
        return IN_WRITE;
    }
    if (strcmp(instruccion, "READ") == 0) {
        *arg1 = strtok(NULL, ":");
        *arg2 = strtok(NULL, "");
        return IN_READ;
    }

    return IN_INVALIDA;
}

void execute_create(char* file_name, char* tag, int conexion_storage) {
    log_info(logger, "Ejecutando CREATE: %s:%s", file_name, tag);
    t_paquete* paquete = crear_paquete();
    paquete->codigo_operacion = CREATE_FILE;
    agregar_a_paquete(paquete, file_name, strlen(file_name) + 1);
    agregar_a_paquete(paquete, tag, strlen(tag) + 1);
    enviar_paquete(paquete, conexion_storage);
    eliminar_paquete(paquete);
    log_info(logger, "Peticion CREATE enviada a Storage.");
}

void execute_truncate(char* file_name, char* tag, int size, int conexion_storage) {
    log_info(logger, "Ejecutando TRUNCATE: %s:%s %d", file_name, tag, size);
    t_paquete* paquete = crear_paquete();
    paquete->codigo_operacion = TRUNCATE_FILE;
    agregar_a_paquete(paquete, file_name, strlen(file_name) + 1);
    agregar_a_paquete(paquete, tag, strlen(tag) + 1);
    agregar_a_paquete(paquete, &size, sizeof(int));
    enviar_paquete(paquete, conexion_storage);
    eliminar_paquete(paquete);
    log_info(logger, "Peticion TRUNCATE enviada a Storage.");
}

void execute_write(char* file_name, char* tag, int base_address, char* content, int conexion_storage) {
    log_info(logger, "Ejecutando WRITE: %s:%s %d %s", file_name, tag, base_address, content);
    // TODO: Implementar la logica de WRITE. 
    // Por ahora, solo se envia la peticion a Storage.
    t_paquete* paquete = crear_paquete();
    paquete->codigo_operacion = WRITE_FILE;
    agregar_a_paquete(paquete, file_name, strlen(file_name) + 1);
    agregar_a_paquete(paquete, tag, strlen(tag) + 1);
    agregar_a_paquete(paquete, &base_address, sizeof(int));
    agregar_a_paquete(paquete, content, strlen(content) + 1);
    enviar_paquete(paquete, conexion_storage);
    eliminar_paquete(paquete);
    log_info(logger, "Peticion WRITE enviada a Storage.");
}

void execute_read(char* file_name, char* tag, int conexion_master, int conexion_storage) {
    log_info(logger, "Ejecutando READ: %s:%s", file_name, tag);

    // La instrucción READ leerá de la Memoria Interna los bytes correspondientes a partir de la dirección base del File y Tag pasados por parámetro,
    // y deberá enviar dicha información al módulo Master.
    // En caso de que la Memoria Interna no cuente con todas las páginas necesarias para satisfacer la operación,
    // deberá solicitar el contenido faltante al módulo Storage.

    // TODO: Implementar la logica de busqueda en Memoria Interna.
    // Por ahora, se asume que no se encuentra en memoria y se solicita a Storage.

    log_info(logger, "Solicitando datos a Storage.");
    t_paquete* paquete_storage = crear_paquete();
    paquete_storage->codigo_operacion = READ_FILE;
    agregar_a_paquete(paquete_storage, file_name, strlen(file_name) + 1);
    agregar_a_paquete(paquete_storage, tag, strlen(tag) + 1);
    enviar_paquete(paquete_storage, conexion_storage);
    eliminar_paquete(paquete_storage);
    log_info(logger, "Peticion READ enviada a Storage.");

    // Esperar respuesta de Storage
    op_code op = recibir_operacion(conexion_storage);
    if (op == FILE_CONTENT) {
        int size;
        void* buffer = recibir_buffer(&size, conexion_storage);
        log_info(logger, "Datos recibidos de Storage.");

        // TODO: Guardar datos en Memoria Interna.

        // Enviar datos a Master
        t_paquete* paquete_master = crear_paquete();
        paquete_master->codigo_operacion = READ_RESULT;
        agregar_a_paquete(paquete_master, buffer, size);
        enviar_paquete(paquete_master, conexion_master);
        eliminar_paquete(paquete_master);
        log_info(logger, "Datos de READ enviados a Master.");
        free(buffer);
    } else {
        log_error(logger, "Error al recibir datos de Storage para READ. Codigo de operacion: %d", op);
        // TODO: Informar error a Master.
    }
}

void activar_QI(int conexion_master, int conexion_storage){
    worker_activo = true;
    while (worker_activo){
        // simulando la rececpcion de la instruccion Truncate
        // ver logica del master?
        char* operacion_recibida = strdup("READ mi_archivo:mi_tag");
// char* operacion_recibida = strdup("WRITE mi_archivo:mi_tag 1024 'este es el contenido'");

        if (operacion_recibida != NULL){
            char* arg1;
            char* arg2;
            char* arg3;
            char* arg4;
            t_instruccion_worker instruccion = parsear_instruccion(operacion_recibida, &arg1, &arg2, &arg3, &arg4);

            switch (instruccion) {
                case IN_CREATE:
                    execute_create(arg1, arg2, conexion_storage);
                    break;
                case IN_TRUNCATE:
                    execute_truncate(arg1, arg2, atoi(arg3), conexion_storage);
                    break;
                case IN_WRITE:
                    execute_write(arg1, arg2, atoi(arg3), arg4, conexion_storage);
                    break;
                case IN_READ:
                    execute_read(arg1, arg2, conexion_master, conexion_storage);
                    break;
                default:
                    log_warning(logger, "Invalid instruction received.");
                    break;
            }
            free(operacion_recibida);
            worker_activo = false; // ejecuta una instruccion y sale...
        }
        else {
            log_error(logger, "Se desconecto el MASTER");
            worker_activo = false;
        }
    }
}
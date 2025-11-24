#include "query_interpreter.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <utils/protocol.h>
#include <utils/clientUtils.h>
#include <utils/serverUtils.h>

bool worker_activo;

typedef enum {
    IN_CREATE,
    IN_TRUNCATE,
    IN_WRITE,
    IN_READ,
    IN_TAG,
    IN_COMMIT,
    IN_FLUSH,
    IN_INVALIDA
} t_instruccion_worker;

// Prototipos de funciones locales
void procesar_query(char* query_path, int pc, int conexion_master, int conexion_storage);
char* get_line(FILE* file, int line_number);
void execute_commit(char* file_name, char* tag, int conexion_storage);
void execute_flush(char* file_name, char* tag, int conexion_storage);

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
    if (strcmp(instruccion, "TAG") == 0) {
        *arg1 = strtok(NULL, ":");
        *arg2 = strtok(NULL, " ");
        *arg3 = strtok(NULL, ":");
        *arg4 = strtok(NULL, "");
        return IN_TAG;
    }
    if (strcmp(instruccion, "COMMIT") == 0) {
        *arg1 = strtok(NULL, ":");
        *arg2 = strtok(NULL, "");
        return IN_COMMIT;
    }
    if (strcmp(instruccion, "FLUSH") == 0) {
        *arg1 = strtok(NULL, ":");
        *arg2 = strtok(NULL, "");
        return IN_FLUSH;
    }

    return IN_INVALIDA;
}

void execute_create(char* file_name, char* tag, int conexion_storage) {
    log_info(logger, "Ejecutando CREATE: %s:%s", file_name, tag);
    t_paquete* paquete = crear_paquete();
    paquete->codigo_operacion = CREATE_FILE;
    int size = 0;
    agregar_a_paquete(paquete, file_name, strlen(file_name) + 1);
    agregar_a_paquete(paquete, tag, strlen(tag) + 1);
    agregar_a_paquete(paquete, &size, sizeof(int));
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

void execute_tag(char* file_name_origen, char* tag_origen, char* file_name_destino, char* tag_destino, int conexion_storage) {
    log_info(logger, "Ejecutando TAG: %s:%s %s:%s", file_name_origen, tag_origen, file_name_destino, tag_destino);
    t_paquete* paquete = crear_paquete();
    paquete->codigo_operacion = TAG_FILE;
    agregar_a_paquete(paquete, file_name_origen, strlen(file_name_origen) + 1);
    agregar_a_paquete(paquete, tag_origen, strlen(tag_origen) + 1);
    agregar_a_paquete(paquete, file_name_destino, strlen(file_name_destino) + 1);
    agregar_a_paquete(paquete, tag_destino, strlen(tag_destino) + 1);
    enviar_paquete(paquete, conexion_storage);
    eliminar_paquete(paquete);
    log_info(logger, "Peticion TAG enviada a Storage.");
}

void execute_commit(char* file_name, char* tag, int conexion_storage) {
    log_info(logger, "Ejecutando COMMIT: %s:%s", file_name, tag);
    execute_flush(file_name, tag, conexion_storage);
    t_paquete* paquete = crear_paquete();
    paquete->codigo_operacion = COMMIT_FILE;
    agregar_a_paquete(paquete, file_name, strlen(file_name) + 1);
    agregar_a_paquete(paquete, tag, strlen(tag) + 1);
    enviar_paquete(paquete, conexion_storage);
    eliminar_paquete(paquete);
    log_info(logger, "Peticion COMMIT enviada a Storage.");
}

void execute_flush(char* file_name, char* tag, int conexion_storage) {
    log_info(logger, "Ejecutando FLUSH: %s:%s", file_name, tag);
    t_paquete* paquete = crear_paquete();
    paquete->codigo_operacion = FLUSH_FILE;
    agregar_a_paquete(paquete, file_name, strlen(file_name) + 1);
    agregar_a_paquete(paquete, tag, strlen(tag) + 1);
    enviar_paquete(paquete, conexion_storage);
    eliminar_paquete(paquete);
    log_info(logger, "Peticion FLUSH enviada a Storage.");
}

void activar_QI(int conexion_master, int conexion_storage) {
    worker_activo = true;
    while (worker_activo) {
        int cod_op = recibir_operacion(conexion_master);
        switch (cod_op) {
            case EXECUTE_QUERY: {
                int size;
                void* buffer = recibir_buffer(&size, conexion_master);
                
                char* query_path = (char*)buffer;
                int pc = *((int*)(buffer + strlen(query_path) + 1));

                log_info(logger, "Recibida solicitud para ejecutar query: %s en PC: %d", query_path, pc);
                procesar_query(query_path, pc, conexion_master, conexion_storage);
                
                free(buffer);
                break;
            }
            case -1:
                log_error(logger, "Se desconecto el MASTER");
                worker_activo = false;
                break;
            default:
                log_warning(logger, "Operacion desconocida de Master. Codigo: %d", cod_op);
                break;
        }
    }
}

void procesar_query(char* query_path, int pc, int conexion_master, int conexion_storage) {
    char full_path[256];
    snprintf(full_path, sizeof(full_path), "%s/%s", path_queries, query_path);

    FILE* query_file = fopen(full_path, "r");
    if (query_file == NULL) {
        log_error(logger, "No se pudo abrir el archivo de query: %s", full_path);
        // TODO: Informar error a Master
        return;
    }

    char* linea = get_line(query_file, pc);
    fclose(query_file);

    if (linea != NULL) {
        log_info(logger, "Ejecutando linea %d: %s", pc, linea);
        char* arg1, *arg2, *arg3, *arg4;
        char* linea_copy = strdup(linea); // Se necesita una copia porque strtok modifica el string
        t_instruccion_worker instruccion = parsear_instruccion(linea_copy, &arg1, &arg2, &arg3, &arg4);

        // Simular retardo
        usleep(retardo_memoria * 1000);

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
            case IN_TAG:
                execute_tag(arg1, arg2, arg3, arg4, conexion_storage);
                break;
            case IN_COMMIT:
                execute_commit(arg1, arg2, conexion_storage);
                break;
            case IN_FLUSH:
                execute_flush(arg1, arg2, conexion_storage);
                break;
            default:
                log_warning(logger, "Instruccion invalida en linea %d: %s", pc, linea);
                // TODO: Informar error a Master
                break;
        }
        free(linea_copy);
        free(linea);
        // TODO: Informar éxito a Master
    } else {
        log_error(logger, "No se pudo leer la linea %d del archivo %s", pc, full_path);
        // TODO: Informar error a Master
    }
}

char* get_line(FILE* file, int line_number) {
    char* line = NULL;
    size_t len = 0;
    ssize_t read;
    int current_line = 0;

    while ((read = getline(&line, &len, file)) != -1) {
        current_line++;
        if (current_line == line_number) {
            // Remover newline al final si existe
            if (line[read - 1] == '\n') {
                line[read - 1] = '\0';
            }
            return line;
        }
    }

    free(line);
    return NULL; // No se encontró la línea
}
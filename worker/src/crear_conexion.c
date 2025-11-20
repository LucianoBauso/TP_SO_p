#include "crear_conexion.h"

void crear_client_worker_storage(){
    char puerto_str[10];
    sprintf(puerto_str, "%d", puerto_storage);
    int conexion_storage = crear_conexion(ip_storage, puerto_str, "WORKER");
        if (conexion_storage == -1) {
            log_error(logger, "No se pudo conectar a Storage en %s:%d", ip_storage, puerto_storage);
            config_destroy(config);
            log_destroy(logger);
            //return EXIT_FAILURE;
            abort();
        }
        log_info(logger, "Conectado a Storage en %s:%d", ip_storage, puerto_storage);

    enviar_mensaje("hola storage, cual es el tamaño del bloque?", conexion_storage);

    // Recibir respuesta
    int cod_op = recibir_operacion(conexion_storage);
    if (cod_op == MENSAJE) {
        int size = 0;
        //char* respuesta = recibir_buffer(&size, conexion_storage);
        //log_info(logger, "Respuesta del Storage: %s", respuesta);
        int* respuesta = recibir_buffer(&size, conexion_storage);
        log_info(logger, "Respuesta del Storage: %d", *respuesta);
        free(respuesta);
    } else {
        log_error(logger, "Operación desconocida recibida: %d", cod_op);
    }
     liberar_conexion(conexion_storage);

}

void crear_client_worker_master(){
    char puerto_str[10];
    sprintf(puerto_str, "%d", puerto_master);
    int conexion_master = crear_conexion(ip_master, puerto_str, "WORKER");
        if (conexion_master == -1) {
            log_error(logger, "No se pudo conectar a Master en %s:%d", ip_master, puerto_master);
            config_destroy(config);
            log_destroy(logger);
            //return EXIT_FAILURE;
            abort();
        }
        log_info(logger, "Conectado a Master en %s:%d", ip_master, puerto_master);
    //char * mensaje_a_enviar = strcat("Hola Master mi id es: ", )
    enviar_mensaje("hola master, mi id es: *inserte id *", conexion_master);

    // Recibir respuesta
    int cod_op = recibir_operacion(conexion_master);
    if (cod_op == MENSAJE) {
        int size = 0;
        //char* respuesta = recibir_buffer(&size, conexion_master);
        //log_info(logger, "Respuesta del Storage: %s", respuesta);
        int* respuesta = recibir_buffer(&size, conexion_master);
        log_info(logger, "Respuesta del Storage: %d", *respuesta);
        free(respuesta);
    } else {
        log_error(logger, "Operación desconocida recibida: %d", cod_op);
    }
     liberar_conexion(conexion_master);

}
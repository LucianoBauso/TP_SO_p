#include "crear_conexion.h"

void crear_client_worker_storage(){
    char puerto_str[10];
    sprintf(puerto_str, "%s", puerto_storage);
    int conexion_storage = crear_conexion(ip_storage, puerto_str, "WORKER");
        if (conexion_storage == -1) {
            log_error(logger, "No se pudo conectar a Storage en %s:%s", ip_storage, puerto_storage);
            config_destroy(config);
            log_destroy(logger);
            //return EXIT_FAILURE;
            abort();
        }
        log_info(logger, "Conectado a Storage en %s:%s", ip_storage, puerto_storage);

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


int conectar_a_master(){ 
    int conexion_master = crear_conexion(ip_master, puerto_master, "WORKER");
        if (conexion_master == -1) {
            log_error(logger, "No se pudo conectar a Master en %s:%s", ip_master, puerto_master);
            config_destroy(config);
            log_destroy(logger);
            //return EXIT_FAILURE;
            abort();
        }
        log_info(logger, "Conectado a Master en %s:%s", ip_master, puerto_master);
        return conexion_master;
}


void enviar_ID_WORKER_a_master(int conexion_master){ // TODO: agregar al header
    int id_worker = 33; //Completamente hardcodeado mal TODO: Armar la logica de generacion de IDs
    log_info(logger,"Quiero mandarle al master el ID de este Worker: %d",id_worker);
    t_paquete* paquete = crear_paquete();
    log_info(logger,"linea 1");
    if (paquete ==NULL)
        {log_info(logger, "EL paquete es null");
    }
    else{
        log_info(logger,"El paquete NO es null  El puntero es %p", (void*)paquete);
    }
    agregar_a_paquete(paquete,&id_worker,sizeof(id_worker));
    log_info(logger,"linea 2");
    enviar_paquete(paquete,conexion_master);
    log_info(logger,"linea 3");
    eliminar_paquete(paquete);
    log_info(logger,"Envie el id al master");

}



//En este caso lo que vamos a mandar es el ID del worker, que es lo que pide el check 1 ;)
void crear_client_worker_master(){ 
    log_info(logger,"Entra en crear_client_worker_master del archivo crear_conexion.c");
    int conexion = conectar_a_master();
    int tipo_cliente = 1; 
    send(conexion, &tipo_cliente, sizeof(tipo_cliente), 0);

    enviar_ID_WORKER_a_master(conexion);

    liberar_conexion(conexion);
    log_info(logger, "Conexión con Master cerrada");
}
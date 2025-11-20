#include "crear_conexion.h"

int conectar_a_storage(){ 
    int conexion_storage= crear_conexion(ip_storage, puerto_storage, "STORAGE");
        if (conexion_storage == -1) {
            log_error(logger, "No se pudo conectar a Storage en %s:%s", ip_storage, puerto_storage);
            config_destroy(config);
            log_destroy(logger);
            //return EXIT_FAILURE;
            abort();
        }
        log_info(logger, "Conectado a Storage en %s:%s", ip_storage, puerto_storage);
        return conexion_storage;
}

void crear_client_worker_storage(){
    int conexion_storage = conectar_a_storage();
    enviar_pedido_BLOCKSIZE_a_storage(conexion_storage);
    

    // Recibir respuesta
    int cod_op = recibir_operacion(conexion_storage);
    if (cod_op == MENSAJE) {
        int size = 0;
        int* respuesta = recibir_buffer(&size, conexion_storage);
        log_info(logger, "Respuesta del Storage: %d", *respuesta);
        free(respuesta);
    } else {
        log_error(logger, "Operación desconocida recibida: %d", cod_op);
    }
     liberar_conexion(conexion_storage);
     log_info(logger, "Conexión con Storage cerrada");

}


int conectar_a_master(){ 
    int conexion_master = crear_conexion(ip_master, puerto_master, "MASTER");
        if (conexion_master == -1) {
            log_error(logger, "No se pudo conectar a Master en %s:%s", ip_master, puerto_master);
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

void enviar_pedido_BLOCKSIZE_a_storage(int conexion_storage){
    log_info(logger, "Preguntando a Storage el tamaño de los bloques");
    enviar_mensaje("Hola storage, cual es el tamaño del bloque?", conexion_storage);
    log_info(logger, "Pregunta enviada a Storage");

}

void enviar_ID_WORKER_a_master(int conexion_master, int id_worker){ 
    log_info(logger,"Quiero mandarle al master el ID de este Worker: %d",id_worker);
    t_paquete* paquete = crear_paquete();

    agregar_a_paquete(paquete,&id_worker,sizeof(id_worker));
    enviar_paquete(paquete,conexion_master);
    eliminar_paquete(paquete);
    log_info(logger,"Envie el id %d al master",id_worker);

}



//En este caso lo que vamos a mandar es el ID del worker, que es lo que pide el check 1 ;)
void crear_client_worker_master(int id){ 
    

    int conexion = conectar_a_master();
    int tipo_cliente = 1; 
    send(conexion, &tipo_cliente, sizeof(tipo_cliente), 0);

    enviar_ID_WORKER_a_master(conexion,id);

    liberar_conexion(conexion);
    log_info(logger, "Conexión con Master cerrada");
}
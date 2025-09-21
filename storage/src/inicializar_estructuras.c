#include "inicializar_estructuras.h"

t_log* logger;
t_config* configStorage;
t_config* configSuperBlock;

    int puerto_escucha;
    bool fresh_start;
    char * punto_montaje;
    int retardo_operacion;
    int retardo_acceso_bloque;
    int FS_SIZE;
    int BLOCK_SIZE;
    t_log_level log_level; 
    char* path;

t_log* iniciar_logger(void) {
	t_log* nuevo_logger = log_create("storage.log", "STORAGE", 1, log_level);
	if (nuevo_logger==NULL){
		perror("Error al crear el logger");
		abort();	
	}

	return nuevo_logger;
}
/*Creacion de storage.config*/
t_config* iniciar_config(path) {
	t_config* nuevo_config = config_create(path);
	if (nuevo_config == NULL){
		perror("Error al crear el config");
		abort();
	}

	return nuevo_config;
}


void leer_configStorage(t_config* configStorage){


    puerto_escucha = config_get_int_value(configStorage, "PUERTO_ESCUCHA");
    char * valor = config_get_string_value(configStorage, "FRESH_START");

    if (strcmp(valor, "TRUE") == 0) {
        fresh_start = true;
    } else if (strcmp(valor, "FALSE") == 0) {
        fresh_start = false;
    } else {
        perror("Valor incorrecto en config.");
    }
    punto_montaje = config_get_string_value(configStorage, "PUNTO_MONTAJE");
    retardo_operacion=config_get_int_value(configStorage, "RETARDO_OPERACION");
    retardo_acceso_bloque = config_get_int_value(configStorage, "RETARDO_ACCESO_BLOQUE");
    log_level = log_level_from_string(config_get_string_value(configStorage, "LOG_LEVEL"));
}

void leer_configSuperBlock(t_config* configSuperBlock){

    if (strcmp(valor, "TRUE") == 0) {
        fresh_start = true;
    } else if (strcmp(valor, "FALSE") == 0) {
        fresh_start = false;
    } else {
        perror("Valor incorrecto en config.");
    }

    FS_SIZE=config_get_int_value(configStorage, "FS_SIZE");
    BLOCK_SIZE=config_get_int_value(configStorage, "BLOCK_SIZE");
}

void inicializar_storage(void){

    configStorage = iniciar_config("storage.config");
    leer_configStorage(configStorage);
    configSuperBlock = iniciar_config("superblock.config");
    leer_configSuperBlock(configSuperBlock);
    logger = iniciar_logger();
    log_info(logger, "Creado Logger de Storage");
    // Esto es solo para probar que config y logger anden, despues se borra
    //log_info(logger, "si lo que quiero loguear abajo loggea, creo que anda todo bien esto entonces: ");
    //log_info(logger,"el valor del punto de montaje es: %s", punto_montaje);

}
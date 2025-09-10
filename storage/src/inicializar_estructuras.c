#include "inicializar_estructuras.h"

t_log* logger;
t_config* config;

    int puerto_escucha;
    bool fresh_start;
    char * punto_montaje;
    int retardo_operacion;
    int retardo_acceso_bloque;
    char * log_level; 

t_log* iniciar_logger(void) {
	t_log* nuevo_logger = log_create("storage.log", "STORAGE", 1, LOG_LEVEL_INFO);
	if (nuevo_logger==NULL){
		perror("Error al crear el logger");
		abort();	
	}

	return nuevo_logger;
}

t_config* iniciar_config(void) {
	t_config* nuevo_config = config_create("storage.config");
	if (nuevo_config == NULL){
		perror("Error al crear el config");
		abort();
	}

	return nuevo_config;
}

void leer_config(t_config* config){


    puerto_escucha = config_get_int_value(config, "PUERTO_ESCUCHA");
    char * valor = config_get_string_value(config, "FRESH_START");

    if (strcmp(valor, "TRUE") == 0) {
        fresh_start = true;
    } else if (strcmp(valor, "FALSE") == 0) {
        fresh_start = false;
    } else {
        perror("Valor incorrecto en config.");
    }
    punto_montaje = config_get_string_value(config, "PUNTO_MONTAJE");
    retardo_operacion=config_get_int_value(config, "RETARDO_OPERACION");
    retardo_acceso_bloque = config_get_int_value(config, "RETARDO_ACCESO_BLOQUE");
    log_level = config_get_string_value(config, "LOG_LEVEL");
}

void inicializar_storage(void){
    logger = iniciar_logger();
    log_info(logger, "Creado Logger de Storage");
    config = iniciar_config();
    leer_config(config);
    // Esto es solo para probar que config y logger anden, despues se borra
    //log_info(logger, "si lo que quiero loguear abajo loggea, creo que anda todo bien esto entonces: ");
    //log_info(logger,"el valor del punto de montaje es: %s", punto_montaje);

}
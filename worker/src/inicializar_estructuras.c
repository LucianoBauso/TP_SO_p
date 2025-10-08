#include "inicializar_estructuras.h"

t_log* logger;
t_config* config;

char * ip_master;
int puerto_master;
char * ip_storage;
int puerto_storage;
int tam_memoria;
int retardo_memoria;
char* algoritmo_reemplazo;
char* path_queries;
char * path;


t_log_level log_level;

t_log* iniciar_logger(void) {
	t_log* nuevo_logger = log_create("worker.log", "WORKER", 1, log_level);
	if (nuevo_logger==NULL){
		perror("Error al crear el logger");
		abort();	
	}

	return nuevo_logger;
}

t_config* iniciar_config(char* path) {
	t_config* nuevo_config = config_create(path);
	if (nuevo_config == NULL){
		perror("Error al crear el config");
		exit(EXIT_FAILURE);
	}

	return nuevo_config;
}

void leer_config(t_config* config){
    ip_master = config_get_string_value(config, "IP_MASTER");
    puerto_master = config_get_int_value(config, "PUERTO_MASTER");
    ip_storage = config_get_string_value(config, "IP_STORAGE");
    puerto_storage = config_get_int_value(config, "PUERTO_STORAGE");
    tam_memoria = config_get_int_value(config, "TAM_MEMORIA");
    retardo_memoria = config_get_int_value(config, "RETARDO_MEMORIA");
    algoritmo_reemplazo = config_get_string_value(config, "ALGORITMO_REEMPLAZO");
    path_queries = config_get_string_value(config, "PATH_QUERIES");
    log_level= log_level_from_string(config_get_string_value(config, "LOG_LEVEL"));
}


void inicializar_worker(void){

    config = iniciar_config("worker.config");
    leer_config(config);
    logger = iniciar_logger();
    log_info(logger, "Creado Logger de Worker");
    // Esto es solo para probar que config y logger anden, despues se borra
    //log_info(logger, "si lo que quiero loguear abajo loggea, creo que anda todo bien esto entonces: ");
    //log_info(logger,"el valor del alg reemplazo es: %s", algoritmo_reemplazo);

}

#include "main.h"

int main(int argc, char* argv[]) {
    if (argc <2){
        perror("No hay parametros suficientes para iniciar Master. Necesito path config. ");
        exit(EXIT_FAILURE);
    }
    inicializar_master();  // crea estructuras
    ejecutar_master(); // crea conexiones ¿
    terminar_master(logger, config);
    return 0;
}



void terminar_master( t_log* logger, t_config* config){ //Deberia llamar tambien a los sockets y liberarlos ??
	log_destroy(logger);
	config_destroy(config);	
}
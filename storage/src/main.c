#include <commons/log.h>
#include <commons/config.h>
#include <utils/serverUtils.h>
#include <utils/clientUtils.h>
#include <utils/protocol.h>  
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <storage.h>
// Provisorio hasta que uses el enum de utils
#define MENSAJE 1

int main(int argc, char* argv[]) {
    if (argc < 2){
        perror("Se espera archivo de configuración"); 
        exit(EXIT_FAILURE);
    }
    char* path_config = argv[1];
    inicializar_storage(path_config);
    
   // ----------- SERVER -----------
    crear_server_storage();
    
    //_____funcionalidad del storage ---------

    // ----------- CLEANUP -----------
    terminar_programa(logger, configStorage, configSuperBlock);
    return 0;
}


void terminar_programa( t_log* logger, t_config* config1, t_config* config2){ //Deberia llamar tambnien a los sockets y liberarlos
	log_destroy(logger);
	config_destroy(config1);
    config_destroy(config2);
	
}
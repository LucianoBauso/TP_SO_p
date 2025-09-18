#include "storage.h"
#include "inicializar_estructuras.h"

int main(int argc, char* argv[]) {


    inicializar_storage();


    terminar_programa(logger, config);
    return 0;
}


void terminar_programa( t_log* logger, t_config* config){ //Deberia llamar tambnien a los sockets y liberarlos
	log_destroy(logger);
	config_destroy(config);
	//liberar_conexion(conexion);
}
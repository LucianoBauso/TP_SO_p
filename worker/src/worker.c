#include <utils/clientUtils.h>
#include <utils/serverUtils.h>
#include <worker.h>
#include <commons/log.h>
#include <commons/config.h>
#include <utils/protocol.h>  
#include <stdlib.h>

#include <inicializar_estructuras.h>
#include <crear_conexion.h>

// Provisorio hasta usar el enum oficial de utils
#define MENSAJE 1

int main(int argc, char* argv[]) {

    inicializar_worker();

    //crear_client_worker_storage(); 
    crear_client_worker_master(); 
    
    //Cerrar programa
   
    config_destroy(config);
    log_destroy(logger);

    return 0;

}

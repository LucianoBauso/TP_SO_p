#include <utils/clientUtils.h>
#include <utils/serverUtils.h>
#include <worker.h>
#include <commons/log.h>
#include <commons/config.h>
#include <utils/protocol.h>  
#include <stdlib.h>



// Provisorio hasta usar el enum oficial de utils
#define MENSAJE 1

//worker.c
int main(int argc, char* argv[]) {

    if (argc != 3) {
        perror("Error: Se esperan exactamente dos parámetros.\n");
        printf("Uso: %s <config> <id worker>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    char* path_config = argv[1];
    int id_actual = atoi(argv[2]);
    inicializar_worker(path_config); 
    crear_client_worker_storage(); 
    int conexion_master = conectar_a_master(id_actual); 
    enviar_ID_WORKER_a_master (conexion_master, id_actual);
    log_info(logger, "Worker (ID: %d) listo y esperando querys...", id_actual);


    //Lógica del modulo
    
    activar_QI(conexion_master); //TODO: Cambiar acá los parametros y el tipo que sea necesario. 

    //Cerrar programa
    config_destroy(config);
    log_destroy(logger);

    return 0;

}

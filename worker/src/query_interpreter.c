//query_interpreter.c
#include "query_interpreter.h"

bool worker_activo;

void activar_QI(int conexion_master){
    worker_activo = true;
    while (worker_activo){
        char* operacion_recibida = recibir_operacion_master(); //TODO: implementar.
        //Tiene que bloquear al worker hasta que reciba esto del master.
        //Las opciones son: recibir o desalojar query. o error qcy
        
        if (operacion_recibida == "RECIBIR QUERY"){ //TODO : corregir condicion
            // TODO: implementar
        }
        else if (operacion_recibida == "DESALOJAR QUERY"){
            //TODO: implementar
        }
        else if (operacion_recibida == -1) { // TODO hablar con master el codigo de desconexion
            log_error(logger, "Se desconecto el MASTER");
            worker_activo = false;
            
        }
        else{
            log_warning(logger, "Master envió un código de operación desconocido.");
            worker_activo = false;
        }
    }


}
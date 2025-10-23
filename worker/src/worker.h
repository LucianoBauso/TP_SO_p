#ifndef CLIENT_H_
#define CLIENT_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include<string.h>

#include "inicializar_estructuras.h"
#include "crear_conexion.h"
#include "query_interpreter.h"



t_log* iniciar_logger(void);
void leer_consola(t_log*);
void paquete(int);
void terminar_programa(int, t_log*, t_config*);

void activar_QI(conexion_master); //TODO : arreglar c parametros q surjan ¿ 

#endif /* CLIENT_H_ */
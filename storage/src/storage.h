#ifndef STORAGE_H_
#define STORAGE_H_

#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>

#include <stdbool.h>
#include <string.h>

#include<inicializar_estructuras.h>

void crear_server_storage();
void terminar_programa(t_log*, t_config*, t_config*); //Seguro haya que cambiarlo para agregar los sockets


#endif
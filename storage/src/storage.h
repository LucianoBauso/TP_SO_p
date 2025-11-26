#ifndef STORAGE_H_
#define STORAGE_H_

#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>

#include <stdbool.h>
#include <string.h>

#include "inicializar_estructuras.h"
#include "crear_conexion.h"
#include "filesystem.h"

void crear_server_storage(void);
void terminar_programa(t_log* logger, t_config* config_storage, t_config* config_superblock);

#endif
#ifndef COMUNICACION_H
#define COMUNICACION_H

#include "utils/clientUtils.h"
#include <string.h>
#include <commons/config.h>
#include <commons/log.h>
#include "main.h"
#include "argumentos.h"

int conectar_a_master(t_config* config, t_log* logger);
void enviar_solicitud_query(int conexion, t_argumentos* args, t_log* logger);

#endif

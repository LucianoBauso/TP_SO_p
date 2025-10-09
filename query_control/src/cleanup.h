#ifndef CLEANUP_H
#define CLEANUP_H

#include <stdlib.h>
#include <commons/log.h>
#include <commons/config.h>
#include "argumentos.h"
#include "configuracion.h"


void cleanup_recursos(t_argumentos* args, t_config_query* config, t_log* logger, t_config* config_commons);

#endif
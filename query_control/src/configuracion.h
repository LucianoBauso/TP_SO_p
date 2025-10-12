#ifndef CONFIGURACION_H
#define CONFIGURACION_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <commons/config.h>
#include <commons/log.h>
#include "main.h"

t_config* iniciar_config(char* archivo_config);

void leer_config_qc (t_config* config_qc );
t_log *iniciar_logger(void);
#endif
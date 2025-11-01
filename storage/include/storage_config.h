#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    char*   ip_escucha;
    int     puerto_escucha;
    bool    fresh_start;
    char*   punto_montaje;
    int     retardo_operacion_ms;
    int     retardo_acceso_bloque_ms;
    char*   log_level;
} storage_cfg_t;

storage_cfg_t* storage_cfg_load(const char* path_cfg);
void storage_cfg_destroy(storage_cfg_t* cfg);
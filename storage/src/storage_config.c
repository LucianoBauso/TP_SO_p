#define _POSIX_C_SOURCE 200809L
#include "storage_config.h"
#include <commons/config.h>
#include <commons/string.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static bool _to_bool(const char* s) {
    if (!s) return false;
    char* lower = string_duplicate(s);
    for (char* p = lower; *p; ++p) *p = (char)tolower(*p);
    bool v = (strcmp(lower, "true")==0 || strcmp(lower, "yes")==0 || strcmp(lower, "1")==0);
    free(lower);
    return v;
}

storage_cfg_t* storage_cfg_load(const char* path_cfg) {
    t_config* cfg = config_create((char*)path_cfg);
    if (!cfg) return NULL;

    storage_cfg_t* out = calloc(1, sizeof(storage_cfg_t));

    const char* ip = config_get_string_value(cfg, "IP_ESCUCHA");
    out->ip_escucha = ip ? string_duplicate(ip) : string_duplicate("0.0.0.0");

    out->puerto_escucha = config_get_int_value(cfg, "PUERTO_ESCUCHA");
    out->fresh_start    = _to_bool(config_get_string_value(cfg, "FRESH_START"));

    const char* pm = config_get_string_value(cfg, "PUNTO_MONTAJE");
    out->punto_montaje = pm ? string_duplicate(pm) : NULL;

    out->retardo_operacion_ms     = config_get_int_value(cfg, "RETARDO_OPERACION");
    out->retardo_acceso_bloque_ms = config_get_int_value(cfg, "RETARDO_ACCESO_BLOQUE");

    const char* ll = config_get_string_value(cfg, "LOG_LEVEL");
    out->log_level = ll ? string_duplicate(ll) : string_duplicate("INFO");

    config_destroy(cfg);
    return out;
}

void storage_cfg_destroy(storage_cfg_t* cfg) {
    if (!cfg) return;
    free(cfg->ip_escucha);
    free(cfg->punto_montaje);
    free(cfg->log_level);
    free(cfg);
}
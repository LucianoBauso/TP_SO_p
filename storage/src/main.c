#define _POSIX_C_SOURCE 200809L
#include "storage_config.h"
#include "fs.h"
#include "server.h"
#include "logger.h"

#include <commons/log.h>
#include <stdio.h>
#include <stdlib.h>

// Logger global del módulo (declarado en logger.h)
t_log* storage_log = NULL;

// commons trae log_level_from_string(); lo envolvemos por si viene NULL
static t_log_level _level_from_cfg(const char* s) {
    return log_level_from_string((char*)(s ? s : "INFO"));
}

static void terminar_programa(storage_cfg_t* cfg) {
    if (storage_log) {
        log_info(storage_log, "Finalizando Storage");
        log_destroy(storage_log);
        storage_log = NULL;
    }
    storage_cfg_destroy(cfg);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <ruta_storage.cfg>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // 1) Cargar configuración
    const char* path_config = argv[1];
    storage_cfg_t* cfg = storage_cfg_load(path_config);
    if (!cfg) {
        fprintf(stderr, "No pude leer config: %s\n", path_config);
        return EXIT_FAILURE;
    }

    // 2) Crear logger con nivel desde config
    storage_log = log_create("storage.log", "STORAGE", true, _level_from_cfg(cfg->log_level));
    if (!storage_log) {
        fprintf(stderr, "No pude crear logger storage.log\n");
        storage_cfg_destroy(cfg);
        return EXIT_FAILURE;
    }

    log_info(storage_log, "Iniciando Storage con config: %s", path_config);

    // 3) Cargar superbloque (<PUNTO_MONTAJE>/superblock.config)
    char* sb_path = fs_path_superblock_cfg(cfg->punto_montaje);
    superblock_t sb = {0};
    if (!superblock_load(sb_path, &sb)) {
        log_error(storage_log, "No pude leer superbloque en %s", sb_path);
        free(sb_path);
        terminar_programa(cfg);
        return EXIT_FAILURE;
    }
    free(sb_path);

    log_info(storage_log, "Superbloque: FS_SIZE=%llu BLOCK_SIZE=%u BLOCKS=%u",
            (unsigned long long)sb.fs_size_bytes, sb.block_size_bytes, sb.blocks_count);

    // 4) Si FRESH_START=TRUE, formatear estructura completa del FS
    if (cfg->fresh_start) {
        int err = 0;
        if (!fs_format_fresh_start(cfg->punto_montaje, &sb, &err)) {
            log_error(storage_log, "FRESH_START falló (errno=%d). Abortando.", err);
            terminar_programa(cfg);
            return EXIT_FAILURE;
        }
        log_info(storage_log, "FRESH_START OK en %s", cfg->punto_montaje);
    }

    // 5) Levantar servidor y atender Workers (threads por conexión)
    server_set_context(cfg, &sb);
    int rc = crear_server_storage();

    // 6) Cleanup
    terminar_programa(cfg);
    return (rc == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

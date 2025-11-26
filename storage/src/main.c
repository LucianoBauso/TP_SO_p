#include "storage.h"
#include "inicializar_estructuras.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <ruta-storage.config>\n", argv[0]);
        return EXIT_FAILURE;
    }

    inicializar_storage(argv[1]);

    crear_server_storage();

    fs_destruir();
    terminar_programa(logger, configStorage, configSuperBlock);

    return EXIT_SUCCESS;
}

void terminar_programa(t_log* logger_local, t_config* config_storage, t_config* config_superblock) {
    if (logger_local != NULL) {
        log_destroy(logger_local);
    }
    if (config_storage != NULL) {
        config_destroy(config_storage);
    }
    if (config_superblock != NULL) {
        config_destroy(config_superblock);
    }
}
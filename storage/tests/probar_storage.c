// storage/tests/probar_storage.c

#include "storage.h"
#include "filesystem.h"
#include "inicializar_estructuras.h"
#include "logger.h"

#include <commons/string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Definimos el logger global que usan fs_init.c, server.c, ops_mock.c
//t_log* storage_log = NULL;

static void probar_create_truncate_write_read(void) {
    int query_id = 1;
    const char* file = "MATERIAS";
    const char* tag  = "BASE";

    printf(">>> Probando CREATE %s:%s\n", file, tag);
    fs_result_t r = fs_crear_file_tag(file, tag, query_id);
    if (r != FS_OK) {
        printf("   ERROR en fs_crear_file_tag (codigo=%d)\n", r);
        return;
    }

    // Truncar a 2 bloques
    uint32_t nuevo_tamanio = BLOCK_SIZE * 2;
    printf(">>> Probando TRUNCATE %s:%s a %u bytes\n", file, tag, nuevo_tamanio);
    r = fs_truncar(file, tag, nuevo_tamanio, query_id);
    if (r != FS_OK) {
        printf("   ERROR en fs_truncar (codigo=%d)\n", r);
        return;
    }

    // Escribimos en bloque lógico 0 todo 'A'
    char* buffer = malloc(BLOCK_SIZE);
    if (!buffer) {
        perror("malloc");
        return;
    }
    memset(buffer, 'A', BLOCK_SIZE);

    printf(">>> Probando WRITE bloque logico 0 con 'A'\n");
    r = fs_escribir_bloque(file, tag, 0, buffer, query_id);
    if (r != FS_OK) {
        printf("   ERROR en fs_escribir_bloque (codigo=%d)\n", r);
        free(buffer);
        return;
    }

    // Escribimos en bloque lógico 1 todo 'B'
    memset(buffer, 'B', BLOCK_SIZE);
    printf(">>> Probando WRITE bloque logico 1 con 'B'\n");
    r = fs_escribir_bloque(file, tag, 1, buffer, query_id);
    if (r != FS_OK) {
        printf("   ERROR en fs_escribir_bloque (codigo=%d)\n", r);
        free(buffer);
        return;
    }

    // Leemos bloque lógico 0 y verificamos contenido
    char* read_buf = malloc(BLOCK_SIZE);
    if (!read_buf) {
        perror("malloc");
        free(buffer);
        return;
    }

    printf(">>> Probando READ bloque logico 0\n");
    r = fs_leer_bloque(file, tag, 0, read_buf, query_id);
    if (r != FS_OK) {
        printf("   ERROR en fs_leer_bloque (codigo=%d)\n", r);
    } else {
        printf("   Primeros 16 bytes bloque 0: ");
        fwrite(read_buf, 1, 16, stdout);
        printf("\n");
    }

    free(buffer);
    free(read_buf);

    printf(">>> Prueba CREATE/TRUNCATE/WRITE/READ terminada\n");
}

int main(void) {
    // Inicializamos Storage con el mismo config que usa bin/storage
    inicializar_storage("storage.config");

    // Hacemos que storage_log apunte al mismo logger global
    storage_log = logger;

    // En este punto, fs_inicializar() ya se llamó dentro de inicializar_storage()
    // (según tu implementación de iniciar_estructuras.c)

    probar_create_truncate_write_read();

    // Limpieza básica (no usamos terminar_programa porque no está linkeado main.c)
    fs_destruir();

    if (logger) {
        log_destroy(logger);
        logger = NULL;
    }
    if (configStorage) {
        config_destroy(configStorage);
        configStorage = NULL;
    }
    if (configSuperBlock) {
        config_destroy(configSuperBlock);
        configSuperBlock = NULL;
    }

    return 0;
}

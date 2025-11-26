#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include <commons/bitarray.h>
#include <commons/log.h>
#include <commons/config.h>
#include <stdint.h>
#include <stdbool.h>

// Bitmap
extern t_bitarray* bitmap;
extern int cant_bloques;

typedef enum {
    FS_OK = 0,
    FS_ERR_FILE_INEXISTENTE,
    FS_ERR_TAG_INEXISTENTE,
    FS_ERR_ESPACIO_INSUFICIENTE,
    FS_ERR_ESCRITURA_NO_PERMITIDA,
    FS_ERR_FUERA_DE_RANGO,
    FS_ERR_ERROR_INTERNO
} fs_result_t;

void fs_inicializar(void);
void fs_destruir(void);

fs_result_t fs_crear_file_tag(const char* file, const char* tag, int query_id);
fs_result_t fs_truncar(const char* file, const char* tag, uint32_t nuevo_tamanio, int query_id);

// Bloques lógicos
fs_result_t fs_leer_bloque(const char* file, const char* tag, uint32_t nro_bloque_logico, void* buffer_out, int query_id);
fs_result_t fs_escribir_bloque(const char* file, const char* tag, uint32_t nro_bloque_logico, const void* buffer_in, int query_id);

#endif
#include "filesystem.h"
#include "inicializar_estructuras.h"

#include <commons/string.h>
#include <commons/bitarray.h>
#include <commons/config.h>
#include <commons/log.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// --------- Variables internas ---------

t_bitarray* bitmap = NULL;
int cant_bloques = 0;

static char* path_bitmap = NULL;
static char* path_blocks_dir = NULL;
static char* path_files_dir = NULL;
static char* path_hash_index = NULL;

// --------- Helpers de paths ---------

static char* fs_path_join(const char* a, const char* b) {
    char* path = string_from_format("%s/%s", a, b);
    return path;
}

static void crear_directorio_si_no_existe(const char* path) {
    struct stat st;
    if (stat(path, &st) == -1) {
        if (mkdir(path, 0777) == -1 && errno != EEXIST) {
            log_error(logger, "Error creando directorio %s: %s", path, strerror(errno));
            perror("mkdir");
        }
    }
}

// --------- Helpers bitmap ---------

static void bitmap_persistir() {
    if (!bitmap || !path_bitmap) return;

    FILE* f = fopen(path_bitmap, "rb+");
    if (!f) {
        f = fopen(path_bitmap, "wb+");
        if (!f) {
            log_error(logger, "No pude abrir bitmap.bin para persistir");
            return;
        }
    }

    fwrite(bitmap->bitarray, bitmap->size, 1, f);
    fflush(f);
    fclose(f);
}

static void bitmap_inicializar_fresh_start() {
    int bytes_bitmap = (cant_bloques + 7) / 8;
    void* data = calloc(bytes_bitmap, 1);
    bitmap = bitarray_create_with_mode(data, bytes_bitmap, LSB_FIRST);

    // Bloque 0 reservado (initial_file)
    bitarray_set_bit(bitmap, 0);

    bitmap_persistir();
}

static void bitmap_cargar_existente() {
    FILE* f = fopen(path_bitmap, "rb");
    if (!f) {
        log_error(logger, "No se encontró bitmap.bin en %s", path_bitmap);
        return;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    void* data = malloc(size);
    fread(data, size, 1, f);
    fclose(f);

    bitmap = bitarray_create_with_mode(data, size, LSB_FIRST);
}

static int reservar_bloque_fisico(int query_id) {
    for (int i = 0; i < cant_bloques; i++) {
        if (!bitarray_test_bit(bitmap, i)) {
            bitarray_set_bit(bitmap, i);
            bitmap_persistir();
            log_info(logger, "##%d - Bloque Físico Reservado - Número de Bloque: %d", query_id, i);
            return i;
        }
    }
    return -1;
}

static void liberar_bloque_fisico(int nro_bloque, int query_id) {
    if (nro_bloque <= 0 || nro_bloque >= cant_bloques) return; // nunca liberamos bloque 0

    if (bitarray_test_bit(bitmap, nro_bloque)) {
        bitarray_clean_bit(bitmap, nro_bloque);
        bitmap_persistir();
        log_info(logger, "##%d - Bloque Físico Liberado - Número de Bloque: %d", query_id, nro_bloque);
    }
}

// --------- Paths de bloques y metadata ---------

static char* path_physical_block(int nro_bloque) {
    char* filename = string_from_format("block%04d.dat", nro_bloque);
    char* path = fs_path_join(path_blocks_dir, filename);
    free(filename);
    return path;
}

static char* path_file_dir(const char* file) {
    return fs_path_join(path_files_dir, file);
}

static char* path_tag_dir(const char* file, const char* tag) {
    char* file_dir = path_file_dir(file);
    char* tag_dir = fs_path_join(file_dir, tag);
    free(file_dir);
    return tag_dir;
}

static char* path_metadata(const char* file, const char* tag) {
    char* tag_dir = path_tag_dir(file, tag);
    char* meta = fs_path_join(tag_dir, "metadata.config");
    free(tag_dir);
    return meta;
}

static char* path_logical_blocks_dir(const char* file, const char* tag) {
    char* tag_dir = path_tag_dir(file, tag);
    char* logical = fs_path_join(tag_dir, "logical_blocks");
    free(tag_dir);
    return logical;
}

static char* path_logical_block(const char* file, const char* tag, uint32_t nro_logico) {
    char* logical_dir = path_logical_blocks_dir(file, tag);
    char* filename = string_from_format("%06u.dat", nro_logico);
    char* path = fs_path_join(logical_dir, filename);
    free(logical_dir);
    free(filename);
    return path;
}

// --------- Helpers de metadata ---------

static bool file_tag_existe(const char* file, const char* tag) {
    char* meta = path_metadata(file, tag);
    bool existe = (access(meta, F_OK) == 0);
    free(meta);
    return existe;
}

static t_config* metadata_abrir(const char* file, const char* tag) {
    char* meta = path_metadata(file, tag);
    if (access(meta, F_OK) != 0) {
        free(meta);
        return NULL;
    }
    t_config* cfg = config_create(meta);
    free(meta);
    return cfg;
}

static void metadata_guardar_y_cerrar(t_config* metadata) {
    if (metadata == NULL) return;
    config_save(metadata);
    config_destroy(metadata);
}

static int* leer_lista_bloques(t_config* metadata, int* cantidad_out) {
    *cantidad_out = 0;
    if (!metadata) return NULL;

    char** array = config_get_array_value(metadata, "BLOCKS");
    if (array == NULL || array[0] == NULL) {
        if (array != NULL) {
            free(array);
        }
        return NULL;
    }

    int count = 0;
    while (array[count] != NULL) {
        count++;
    }

    int* bloques = malloc(sizeof(int) * count);
    for (int i = 0; i < count; i++) {
        bloques[i] = atoi(array[i]);
        free(array[i]);
    }
    free(array);

    *cantidad_out = count;
    return bloques;
}

static void escribir_lista_bloques(t_config* metadata, int* bloques, int cantidad) {
    char* lista = string_new();
    string_append(&lista, "[");

    for (int i = 0; i < cantidad; i++) {
        char* num = string_from_format("%d", bloques[i]);
        string_append(&lista, num);
        free(num);
        if (i < cantidad - 1) {
            string_append(&lista, ",");
        }
    }

    string_append(&lista, "]");
    config_set_value(metadata, "BLOCKS", lista);
    free(lista);
}

// --------- Inicialización del FS ---------

static void crear_blocks_y_files_dirs() {
    crear_directorio_si_no_existe(punto_montaje);
    crear_directorio_si_no_existe(path_blocks_dir);
    crear_directorio_si_no_existe(path_files_dir);
}

static void limpiar_fs_previo() {
    // Implementación simple usando rm -rf sobre las estructuras del FS
    char* cmd = string_from_format("rm -rf '%s/bitmap.bin' '%s/blocks_hash_index.config' '%s/physical_blocks' '%s/files'",
                                  punto_montaje, punto_montaje, punto_montaje, punto_montaje);
    int res = system(cmd);
    if (res == -1) {
        log_error(logger, "Error ejecutando limpieza de FS previa con rm -rf");
    }
    free(cmd);
}

static void crear_archivo_blocks_hash_index() {
    FILE* f = fopen(path_hash_index, "w");
    if (f == NULL) {
        log_error(logger, "No se pudo crear blocks_hash_index.config");
        return;
    }
    fclose(f);
}

static void crear_archivos_blocks() {
    // Creamos todos los bloques físicos con tamaño fijo
    void* buffer_cero = calloc(BLOCK_SIZE, 1);
    void* buffer_cero_char = malloc(BLOCK_SIZE);
    memset(buffer_cero_char, '0', BLOCK_SIZE);

    for (int i = 0; i < cant_bloques; i++) {
        char* path_block = path_physical_block(i);
        FILE* f = fopen(path_block, "wb+");
        if (!f) {
            log_error(logger, "No se pudo crear bloque físico %s", path_block);
            free(path_block);
            continue;
        }

        if (i == 0) {
            fwrite(buffer_cero_char, BLOCK_SIZE, 1, f);
        } else {
            fwrite(buffer_cero, BLOCK_SIZE, 1, f);
        }
        fclose(f);
        free(path_block);
    }

    free(buffer_cero);
    free(buffer_cero_char);
}

static void crear_initial_file() {
    // /files/initial_file/BASE
    char* file_dir = fs_path_join(path_files_dir, "initial_file");
    char* tag_dir = fs_path_join(file_dir, "BASE");
    char* logical_dir = fs_path_join(tag_dir, "logical_blocks");

    crear_directorio_si_no_existe(file_dir);
    crear_directorio_si_no_existe(tag_dir);
    crear_directorio_si_no_existe(logical_dir);

    // metadata.config
    char* meta_path = fs_path_join(tag_dir, "metadata.config");
    FILE* meta = fopen(meta_path, "w");
    if (!meta) {
        log_error(logger, "No se pudo crear metadata de initial_file");
    } else {
        fprintf(meta, "TAMAÑO=%d\n", BLOCK_SIZE);
        fprintf(meta, "BLOCKS=[0]\n");
        fprintf(meta, "ESTADO=WORK_IN_PROGRESS\n");
        fclose(meta);
    }

    // logical_blocks/000000.dat -> hard link a block0000.dat
    char* logical_block0 = fs_path_join(logical_dir, "000000.dat");
    char* physical_block0 = path_physical_block(0);

    if (link(physical_block0, logical_block0) == -1) {
        log_error(logger, "Error creando hard link inicial_file BASE: %s", strerror(errno));
    } else {
        log_info(logger, "Hard link inicial initial_file:BASE -> bloque físico 0 creado");
    }

    free(file_dir);
    free(tag_dir);
    free(logical_dir);
    free(meta_path);
    free(logical_block0);
    free(physical_block0);
}

void fs_inicializar(void) {
    // Calculamos cantidad de bloques
    cant_bloques = FS_SIZE / BLOCK_SIZE;

    // Armamos paths base
    path_bitmap = fs_path_join(punto_montaje, "bitmap.bin");
    path_hash_index = fs_path_join(punto_montaje, "blocks_hash_index.config");
    path_blocks_dir = fs_path_join(punto_montaje, "physical_blocks");
    path_files_dir = fs_path_join(punto_montaje, "files");

    if (fresh_start) {
        log_info(logger, "FRESH_START=TRUE -> Formateando FS en %s", punto_montaje);
        limpiar_fs_previo();
        crear_blocks_y_files_dirs();
        bitmap_inicializar_fresh_start();
        crear_archivos_blocks();
        crear_archivo_blocks_hash_index();
        crear_initial_file();
    } else {
        log_info(logger, "FRESH_START=FALSE -> Levantando FS existente en %s", punto_montaje);
        crear_blocks_y_files_dirs();
        bitmap_cargar_existente();
    }
}

void fs_destruir(void) {
    if (bitmap) {
        void* data = bitmap->bitarray;
        bitarray_destroy(bitmap);
        free(data);
        bitmap = NULL;
    }
    free(path_bitmap);
    free(path_blocks_dir);
    free(path_files_dir);
    free(path_hash_index);
}

// --------- Operaciones públicas (CREATE, TRUNCATE, READ, WRITE) ---------

fs_result_t fs_crear_file_tag(const char* file, const char* tag, int query_id) {
    usleep(retardo_operacion * 1000);

    if (file_tag_existe(file, tag)) {
        log_error(logger, "Intento de crear File:Tag ya existente %s:%s", file, tag);
        return FS_ERR_ERROR_INTERNO;
    }

    char* file_dir = path_file_dir(file);
    char* tag_dir = path_tag_dir(file, tag);
    char* logical_dir = fs_path_join(tag_dir, "logical_blocks");
    char* meta_path = fs_path_join(tag_dir, "metadata.config");

    crear_directorio_si_no_existe(file_dir);
    crear_directorio_si_no_existe(tag_dir);
    crear_directorio_si_no_existe(logical_dir);

    FILE* meta = fopen(meta_path, "w");
    if (!meta) {
        log_error(logger, "No se pudo crear metadata para %s:%s", file, tag);
        free(file_dir);
        free(tag_dir);
        free(logical_dir);
        free(meta_path);
        return FS_ERR_ERROR_INTERNO;
    }

    fprintf(meta, "TAMAÑO=0\n");
    fprintf(meta, "BLOCKS=[]\n");
    fprintf(meta, "ESTADO=WORK_IN_PROGRESS\n");
    fclose(meta);

    log_info(logger, "##%d - File Creado %s:%s", query_id, file, tag);

    free(file_dir);
    free(tag_dir);
    free(logical_dir);
    free(meta_path);

    return FS_OK;
}

fs_result_t fs_truncar(const char* file, const char* tag, uint32_t nuevo_tamanio, int query_id) {
    usleep(retardo_operacion * 1000);

    t_config* metadata = metadata_abrir(file, tag);
    if (!metadata) {
        log_error(logger, "TRUNCATE sobre File:Tag inexistente %s:%s", file, tag);
        return FS_ERR_FILE_INEXISTENTE;
    }

    const char* estado = config_get_string_value(metadata, "ESTADO");
    if (estado && strcmp(estado, "COMMITED") == 0) {
        log_error(logger, "TRUNCATE no permitido sobre %s:%s en estado COMMITED", file, tag);
        metadata_guardar_y_cerrar(metadata);
        return FS_ERR_ESCRITURA_NO_PERMITIDA;
    }

    int bloques_actuales_cant = 0;
    int* bloques_actuales = leer_lista_bloques(metadata, &bloques_actuales_cant);

    int bloques_nuevos_cant = nuevo_tamanio / BLOCK_SIZE;

    // Agrandar
    if (bloques_nuevos_cant > bloques_actuales_cant) {
        bloques_actuales = realloc(bloques_actuales, sizeof(int) * bloques_nuevos_cant);
        for (int i = bloques_actuales_cant; i < bloques_nuevos_cant; i++) {
            bloques_actuales[i] = 0; // apuntan al bloque físico 0
            // Crear hard link lógico -> físico 0
            char* logical_path = path_logical_block(file, tag, i);
            char* physical_path = path_physical_block(0);
            if (link(physical_path, logical_path) == -1) {
                log_error(logger, "Error creando hard link en TRUNCATE para %s:%s bloque lógico %d: %s",
                          file, tag, i, strerror(errno));
            } else {
                log_info(logger, "##%d - %s:%s Se agregó el hard link del bloque lógico %d al bloque físico 0",
                         query_id, file, tag, i);
            }
            free(logical_path);
            free(physical_path);
        }
    }

    // Achicar
    if (bloques_nuevos_cant < bloques_actuales_cant) {
        for (int i = bloques_actuales_cant - 1; i >= bloques_nuevos_cant; i--) {
            int bloque_fisico = bloques_actuales[i];

            char* logical_path = path_logical_block(file, tag, i);
            if (unlink(logical_path) == -1) {
                log_error(logger, "Error eliminando hard link lógico %s: %s", logical_path, strerror(errno));
            } else {
                log_info(logger, "##%d - %s:%s Se eliminó el hard link del bloque lógico %d al bloque físico %d",
                         query_id, file, tag, i, bloque_fisico);
            }
            free(logical_path);

            // Si nadie más referencia al bloque físico -> lo liberamos
            char* physical_path = path_physical_block(bloque_fisico);
            struct stat st;
            if (stat(physical_path, &st) == 0) {
                if (st.st_nlink == 1) { // sólo el archivo físico
                    liberar_bloque_fisico(bloque_fisico, query_id);
                }
            }
            free(physical_path);
        }
    }

    // Actualizamos metadata
    if (bloques_nuevos_cant == 0) {
        free(bloques_actuales);
        bloques_actuales = NULL;
    }

    config_set_int_value(metadata, "TAMAÑO", nuevo_tamanio);
    escribir_lista_bloques(metadata, bloques_actuales, bloques_nuevos_cant);

    metadata_guardar_y_cerrar(metadata);
    free(bloques_actuales);

    log_info(logger, "##%d - File Truncado %s:%s - Tamaño: %u", query_id, file, tag, nuevo_tamanio);

    return FS_OK;
}

fs_result_t fs_leer_bloque(const char* file, const char* tag, uint32_t nro_bloque_logico, void* buffer_out, int query_id) {
    usleep(retardo_operacion * 1000);
    usleep(retardo_acceso_bloque * 1000);

    t_config* metadata = metadata_abrir(file, tag);
    if (!metadata) {
        log_error(logger, "READ sobre File:Tag inexistente %s:%s", file, tag);
        return FS_ERR_FILE_INEXISTENTE;
    }

    int cant_bloques = 0;
    int* bloques = leer_lista_bloques(metadata, &cant_bloques);
    if (nro_bloque_logico >= (uint32_t)cant_bloques) {
        metadata_guardar_y_cerrar(metadata);
        free(bloques);
        log_error(logger, "READ fuera de límite en %s:%s bloque lógico %u", file, tag, nro_bloque_logico);
        return FS_ERR_FUERA_DE_RANGO;
    }

    int bloque_fisico = bloques[nro_bloque_logico];
    char* physical_path = path_physical_block(bloque_fisico);

    FILE* f = fopen(physical_path, "rb");
    if (!f) {
        log_error(logger, "No se pudo abrir bloque físico %s para lectura", physical_path);
        metadata_guardar_y_cerrar(metadata);
        free(bloques);
        free(physical_path);
        return FS_ERR_ERROR_INTERNO;
    }

    size_t leidos = fread(buffer_out, 1, BLOCK_SIZE, f);
    fclose(f);
    if (leidos < (size_t)BLOCK_SIZE) {
        // rellenamos con ceros el resto
        memset((char*)buffer_out + leidos, 0, BLOCK_SIZE - leidos);
    }

    log_info(logger, "##%d - Bloque Lógico Leído %s:%s - Número de Bloque: %u", query_id, file, tag, nro_bloque_logico);

    metadata_guardar_y_cerrar(metadata);
    free(bloques);
    free(physical_path);

    return FS_OK;
}

fs_result_t fs_escribir_bloque(const char* file, const char* tag, uint32_t nro_bloque_logico, const void* buffer_in, int query_id) {
    usleep(retardo_operacion * 1000);
    usleep(retardo_acceso_bloque * 1000);

    t_config* metadata = metadata_abrir(file, tag);
    if (!metadata) {
        log_error(logger, "WRITE sobre File:Tag inexistente %s:%s", file, tag);
        return FS_ERR_FILE_INEXISTENTE;
    }

    const char* estado = config_get_string_value(metadata, "ESTADO");
    if (estado && strcmp(estado, "COMMITED") == 0) {
        log_error(logger, "WRITE no permitido sobre %s:%s en estado COMMITED", file, tag);
        metadata_guardar_y_cerrar(metadata);
        return FS_ERR_ESCRITURA_NO_PERMITIDA;
    }

    int cant_bloques = 0;
    int* bloques = leer_lista_bloques(metadata, &cant_bloques);
    if (nro_bloque_logico >= (uint32_t)cant_bloques) {
        metadata_guardar_y_cerrar(metadata);
        free(bloques);
        log_error(logger, "WRITE fuera de límite en %s:%s bloque lógico %u", file, tag, nro_bloque_logico);
        return FS_ERR_FUERA_DE_RANGO;
    }

    int bloque_fisico_actual = bloques[nro_bloque_logico];
    char* physical_path = path_physical_block(bloque_fisico_actual);

    struct stat st;
    if (stat(physical_path, &st) == -1) {
        log_error(logger, "No se pudo hacer stat de %s: %s", physical_path, strerror(errno));
        metadata_guardar_y_cerrar(metadata);
        free(bloques);
        free(physical_path);
        return FS_ERR_ERROR_INTERNO;
    }

    // Si el bloque físico está compartido por más de un hard link, hacemos copy-on-write
    if (st.st_nlink > 1) {
        int nuevo_bloque_fisico = reservar_bloque_fisico(query_id);
        if (nuevo_bloque_fisico == -1) {
            metadata_guardar_y_cerrar(metadata);
            free(bloques);
            free(physical_path);
            return FS_ERR_ESPACIO_INSUFICIENTE;
        }

        char* nueva_ruta_fisica = path_physical_block(nuevo_bloque_fisico);

        // Actualizamos hard link lógico
        char* logical_path = path_logical_block(file, tag, nro_bloque_logico);
        if (unlink(logical_path) == -1) {
            log_error(logger, "Error eliminando hard link anterior %s: %s", logical_path, strerror(errno));
        } else {
            log_info(logger, "##%d - %s:%s Se eliminó el hard link del bloque lógico %u al bloque físico %d",
                     query_id, file, tag, nro_bloque_logico, bloque_fisico_actual);
        }

        if (link(nueva_ruta_fisica, logical_path) == -1) {
            log_error(logger, "Error creando nuevo hard link %s -> %s: %s",
                      logical_path, nueva_ruta_fisica, strerror(errno));
        } else {
            log_info(logger, "##%d - %s:%s Se agregó el hard link del bloque lógico %u al bloque físico %d",
                     query_id, file, tag, nro_bloque_logico, nuevo_bloque_fisico);
        }

        // Liberamos el bloque físico viejo si ya no tiene referencias lógicas
        if (stat(physical_path, &st) == 0 && st.st_nlink == 1) {
            liberar_bloque_fisico(bloque_fisico_actual, query_id);
        }

        bloques[nro_bloque_logico] = nuevo_bloque_fisico;
        free(physical_path);
        physical_path = nueva_ruta_fisica;
        free(logical_path);
    }

    // Escribimos el contenido en el bloque físico
    FILE* f = fopen(physical_path, "rb+");
    if (!f) {
        log_error(logger, "No se pudo abrir bloque físico %s para escritura", physical_path);
        metadata_guardar_y_cerrar(metadata);
        free(bloques);
        free(physical_path);
        return FS_ERR_ERROR_INTERNO;
    }

    fwrite(buffer_in, BLOCK_SIZE, 1, f);
    fflush(f);
    fclose(f);

    // Guardamos la nueva lista de bloques (por si se reasignó el físico)
    escribir_lista_bloques(metadata, bloques, cant_bloques);
    metadata_guardar_y_cerrar(metadata);

    log_info(logger, "##%d - Bloque Lógico Escrito %s:%s - Número de Bloque: %u", query_id, file, tag, nro_bloque_logico);

    free(bloques);
    free(physical_path);

    return FS_OK;
}

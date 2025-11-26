#include "inicializar_estructuras.h"
#include "filesystem.h"

#include <commons/string.h>
#include <stdlib.h>
#include <stdio.h>

t_log* logger = NULL;
t_config* configStorage = NULL;
t_config* configSuperBlock = NULL;

char* puerto_escucha = NULL;
bool fresh_start = false;
char* punto_montaje = NULL;
int retardo_operacion = 0;
int retardo_acceso_bloque = 0;
int FS_SIZE = 0;
int BLOCK_SIZE = 0;
t_log_level log_level = LOG_LEVEL_INFO;

t_log* iniciar_logger(void)
{
    t_log* nuevo_logger = log_create("storage.log", "STORAGE", 1, log_level);
    if (nuevo_logger == NULL)
    {
        perror("Error al crear el logger de Storage");
        exit(EXIT_FAILURE);
    }
    return nuevo_logger;
}

t_config* iniciar_config(char* path)
{
    t_config* nuevo_config = config_create(path);
    if (nuevo_config == NULL)
    {
        fprintf(stderr, "No pude leer el archivo de configuración: %s\n", path);
        exit(EXIT_FAILURE);
    }
    return nuevo_config;
}

void leer_configStorage(t_config* configuracion)
{
    puerto_escucha = config_get_string_value(configuracion, "PUERTO_ESCUCHA");
    fresh_start = config_get_bool_value(configuracion, "FRESH_START");
    punto_montaje = config_get_string_value(configuracion, "PUNTO_MONTAJE");
    retardo_operacion = config_get_int_value(configuracion, "RETARDO_OPERACION");
    retardo_acceso_bloque = config_get_int_value(configuracion, "RETARDO_ACCESO_BLOQUE");
    log_level = log_level_from_string(config_get_string_value(configuracion, "LOG_LEVEL"));
}

void leer_configSuperBlock(t_config* configuracion)
{
    FS_SIZE = config_get_int_value(configuracion, "FS_SIZE");
    BLOCK_SIZE = config_get_int_value(configuracion, "BLOCK_SIZE");
}

void inicializar_storage(char* path_config)
{
    configStorage = iniciar_config(path_config);
    leer_configStorage(configStorage);

    logger = iniciar_logger();
    log_info(logger, "Logger de Storage creado");
    log_info(logger, "PUERTO_ESCUCHA = %s", puerto_escucha);
    log_info(logger, "PUNTO_MONTAJE = %s", punto_montaje);
    log_info(logger, "FRESH_START = %s", fresh_start ? "TRUE" : "FALSE");

    char* path_superblock = string_from_format("%s/superblock.config", punto_montaje);
    configSuperBlock = iniciar_config(path_superblock);
    leer_configSuperBlock(configSuperBlock);
    log_info(logger, "FS_SIZE = %d, BLOCK_SIZE = %d", FS_SIZE, BLOCK_SIZE);
    free(path_superblock);

    fs_inicializar();
}
#include "inicializar_estructuras.h"

t_log *logger;
t_config *configStorage;
t_config *configSuperBlock;

char *puerto_escucha; 
bool fresh_start;
char *punto_montaje;
int retardo_operacion;
int retardo_acceso_bloque;
int FS_SIZE;
int BLOCK_SIZE;
t_log_level log_level;
char *path;

t_log *iniciar_logger(void)
{
    t_log *nuevo_logger = log_create("storage.log", "STORAGE", 1, log_level);
    if (nuevo_logger == NULL)
    {
        perror("Error al crear el logger");
        exit(EXIT_FAILURE);
    }

    return nuevo_logger;
}
/*Creacion de storage.config*/
t_config *iniciar_config(char *path)
{
    t_config *nuevo_config = config_create(path);
    if (nuevo_config == NULL)
    {
        perror("Error al crear el config");
        exit(EXIT_FAILURE);
    }

    return nuevo_config;
}

void leer_configStorage(t_config *configStorage)
{

    puerto_escucha = config_get_string_value(configStorage, "PUERTO_ESCUCHA");
    // PREGUNTAR si esto va en cada clave.
    // Si alcanza con que uno de los valores no esté para que se rompa.
    if (!config_has_property(configStorage, "PUERTO_ESCUCHA"))
    {
        perror("El config no tiene la clave PUERTO_ESCUCHA");
        config_destroy(configStorage);
        exit(EXIT_FAILURE);
    }
    char *valor = config_get_string_value(configStorage, "FRESH_START");

    if (strcmp(valor, "TRUE") == 0)
    {
        fresh_start = true;
    }
    else if (strcmp(valor, "FALSE") == 0)
    {
        fresh_start = false;
    }
    else
    {
        perror("Valor incorrecto en config.");
    }
    punto_montaje = config_get_string_value(configStorage, "PUNTO_MONTAJE");
    retardo_operacion = config_get_int_value(configStorage, "RETARDO_OPERACION");
    retardo_acceso_bloque = config_get_int_value(configStorage, "RETARDO_ACCESO_BLOQUE");
    log_level = log_level_from_string(config_get_string_value(configStorage, "LOG_LEVEL"));
}

void leer_configSuperBlock(t_config *configSuperBlock)
{

    FS_SIZE = config_get_int_value(configSuperBlock, "FS_SIZE");
    BLOCK_SIZE = config_get_int_value(configSuperBlock, "BLOCK_SIZE");
}

void inicializar_storage(char* path_config)
{

    configStorage = iniciar_config(path_config);
    leer_configStorage(configStorage);
    configSuperBlock = iniciar_config("superblock.config");
    leer_configSuperBlock(configSuperBlock);
    logger = iniciar_logger();
    log_info(logger, "Creado Logger de Storage");
    log_info(logger, "Leí puerto_escucha = %s", puerto_escucha);


}
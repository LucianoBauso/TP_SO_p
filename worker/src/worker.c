#include <utils/hello.h>
#include <worker.h>

int main(void) {
    saludar("worker");

    int conexion;
	char* ip_master;
    char* ip_storage;
	int puerto_master;
    int puerto_storage;
	char* valor;

	t_log* logger;
	t_config* config;

    /* ---------------- LOGGING ---------------- */

	logger = iniciar_logger();
    log_info(logger, "Soy Worker");

    /* ---------------- ARCHIVO DE CONFIGURACION ---------------- */

	config = iniciar_config();
    ip_master = config_get_string_value(config, "IP_MASTER");
    ip_storage = config_get_string_value(config, "IP_STORAGE");
	puerto_master = config_get_int_value(config, "PUERTO_MASTER");
    puerto_storage = config_get_int_value(config, "PUERTO_MASTER");
	
}

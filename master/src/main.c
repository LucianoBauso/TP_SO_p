#include "main.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        perror("No hay parámetros suficientes para iniciar Master. Necesito path de configuración.");
        exit(EXIT_FAILURE);
    }
    
    inicializar_master();  // Crea estructuras y lee configuración
    ejecutar_master();     // Crea conexiones y ejecuta el servidor
    finalizar_master(logger, config);
    
    return 0;
}

void finalizar_master(t_log* logger, t_config* configuracion) {
    // Debería también cerrar sockets y liberar recursos
    log_destroy(logger);
    config_destroy(configuracion);	
}
#include "main.h"

int main(int argc, char* argv[]) {
    if (argc <2){
        perror("No hay parametros suficientes para iniciar Master. Necesito path config. ");
        exit(EXIT_FAILURE);
    }
    inicializar_master(); // listop
    //crear_server_master();
    //terminar_master();
    return 0;
}

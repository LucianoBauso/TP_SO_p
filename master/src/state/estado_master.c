#include "estado_master.h"
#include <stdlib.h>
#include <string.h>

t_list* CONEXIONES_QUERY_CONTROL = NULL;
t_list* COLA_READY               = NULL;
t_list* LISTA_EXEC               = NULL;

pthread_mutex_t MUTEX_QUERY_CONTROL = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t MUTEX_READY         = PTHREAD_MUTEX_INITIALIZER;

int PROXIMO_QUERY_ID = 0;

void inicializar_estado_master(void) {
    CONEXIONES_QUERY_CONTROL = list_create();
    COLA_READY               = list_create();
    LISTA_EXEC               = list_create();
}

static t_conexion_query_control* _buscar_conexion_por_socket(int socket_query_control) {
    t_conexion_query_control* encontrada = NULL;
    pthread_mutex_lock(&MUTEX_QUERY_CONTROL);
    for (int i = 0; i < list_size(CONEXIONES_QUERY_CONTROL); i++) {
        t_conexion_query_control* conexion = list_get(CONEXIONES_QUERY_CONTROL, i);
        if (conexion->socket_query_control == socket_query_control) { 
            encontrada = conexion; 
            break; 
        }
    }
    pthread_mutex_unlock(&MUTEX_QUERY_CONTROL);
    return encontrada;
}

t_query* crear_query(int socket_query_control, const char* path_query, int prioridad) {
    t_query* query = malloc(sizeof(t_query));
    query->query_id              = __atomic_fetch_add(&PROXIMO_QUERY_ID, 1, __ATOMIC_SEQ_CST);
    query->prioridad             = prioridad;
    query->path_query            = strdup(path_query ? path_query : "");
    query->socket_query_control  = socket_query_control;
    query->estado                = QUERY_READY;
    return query;
}

void encolar_en_ready(t_query* query) {
    pthread_mutex_lock(&MUTEX_READY);
    list_add(COLA_READY, query);
    pthread_mutex_unlock(&MUTEX_READY);
}

int cantidad_en_ready(void) {
    pthread_mutex_lock(&MUTEX_READY);
    int cantidad = list_size(COLA_READY);
    pthread_mutex_unlock(&MUTEX_READY);
    return cantidad;
}

void asociar_query_a_conexion(int socket_query_control, t_query* query) {
    pthread_mutex_lock(&MUTEX_QUERY_CONTROL);
    for (int i = 0; i < list_size(CONEXIONES_QUERY_CONTROL); i++) {
        t_conexion_query_control* conexion = list_get(CONEXIONES_QUERY_CONTROL, i);
        if (conexion->socket_query_control == socket_query_control) {
            list_add(conexion->queries, query);
            break;
        }
    }
    pthread_mutex_unlock(&MUTEX_QUERY_CONTROL);
}

static void _liberar_query(void* elemento) {
    t_query* query = (t_query*) elemento;
    if (!query) return;
    free(query->path_query);
    free(query);
}

static void _liberar_conexion_query_control(void* elemento) {
    t_conexion_query_control* conexion = (t_conexion_query_control*) elemento;
    if (!conexion) return;
    if (conexion->queries) list_destroy_and_destroy_elements(conexion->queries, _liberar_query);
    free(conexion);
}

void finalizar_estado_master(void) {
    if (CONEXIONES_QUERY_CONTROL) { 
        list_destroy_and_destroy_elements(CONEXIONES_QUERY_CONTROL, _liberar_conexion_query_control); 
        CONEXIONES_QUERY_CONTROL = NULL; 
    }
    if (COLA_READY) { 
        list_destroy_and_destroy_elements(COLA_READY, _liberar_query); 
        COLA_READY = NULL; 
    }
    if (LISTA_EXEC) { 
        list_destroy_and_destroy_elements(LISTA_EXEC, _liberar_query); 
        LISTA_EXEC = NULL; 
    }

    pthread_mutex_destroy(&MUTEX_QUERY_CONTROL);
    pthread_mutex_destroy(&MUTEX_READY);
}

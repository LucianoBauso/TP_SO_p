#include "estado_master.h"
#include <stdlib.h>
#include <string.h>

t_list* CONEXIONES_QUERY_CONTROL = NULL;
t_list* COLA_READY               = NULL;
t_list* LISTA_EXEC               = NULL;

pthread_mutex_t MUTEX_QUERY_CONTROL = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t MUTEX_READY         = PTHREAD_MUTEX_INITIALIZER;

int PROXIMO_QUERY_ID = 0;
e_algoritmo_planificacion ALGORITMO_ACTUAL = ALGO_FIFO; //Valor por defecto, en tiempo de ejecucion puede cambiar segun la config

void inicializar_estado_master(void) {
    CONEXIONES_QUERY_CONTROL = list_create();
    COLA_READY               = list_create();
    LISTA_EXEC               = list_create();
}

void configurar_algoritmo_planificacion(const char* algoritmo_str) {
    if (strcmp(algoritmo_str, "FIFO") == 0) {
        ALGORITMO_ACTUAL = ALGO_FIFO;
    } else if (strcmp(algoritmo_str, "PRIORIDAD") == 0) {
        ALGORITMO_ACTUAL = ALGO_PRIORIDAD;
    } else {
        ALGORITMO_ACTUAL = ALGO_FIFO; // Default
    }
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
    
    switch(ALGORITMO_ACTUAL) {
        case ALGO_FIFO:
            // FIFO: agregar al final
            list_add(COLA_READY, query);
            break;
            
        case ALGO_PRIORIDAD:
            // Prioridad: insertar en posición correcta
            // Mayor prioridad = menor número (1 es más prioritario que 10)
            {
                int posicion = 0;
                for (int i = 0; i < list_size(COLA_READY); i++) {
                    t_query* q_actual = list_get(COLA_READY, i);
                    if (query->prioridad < q_actual->prioridad) {
                        posicion = i;
                        break;
                    }
                    posicion = i + 1;
                }
                list_add_in_index(COLA_READY, posicion, query);
            }
            break;
            
        default:
            list_add(COLA_READY, query);
            break;
    }
    
    pthread_mutex_unlock(&MUTEX_READY);
}

int cantidad_en_ready(void) {
    pthread_mutex_lock(&MUTEX_READY);
    int cantidad = list_size(COLA_READY);
    pthread_mutex_unlock(&MUTEX_READY);
    return cantidad;
}

t_query* obtener_proxima_query_ready(void) {
    t_query* q = NULL;
    pthread_mutex_lock(&MUTEX_READY);
    
    if (list_size(COLA_READY) > 0) {
        // En todos los algoritmos tomamos el primero porque ya está ordenado
        q = list_remove(COLA_READY, 0);
        if (q) {
            q->estado = QUERY_EXEC;
            list_add(LISTA_EXEC, q);
        }
    }
    
    pthread_mutex_unlock(&MUTEX_READY);
    return q;
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

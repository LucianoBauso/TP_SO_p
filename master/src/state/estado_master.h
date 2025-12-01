#ifndef ESTADO_MASTER_H
#define ESTADO_MASTER_H

#include <commons/collections/list.h>
#include <pthread.h>

typedef enum {
    QUERY_READY,
    QUERY_EXEC,
    QUERY_EXIT
} e_estado_query;

typedef enum {
    ALGO_FIFO,
    ALGO_PRIORIDAD
} e_algoritmo_planificacion;

typedef struct {
    int   query_id;
    int   prioridad;
    char* path_query;
    int   socket_query_control;
    e_estado_query estado;
} t_query;

// Representa una conexión de Query Control
typedef struct {
    int     socket_query_control;      // socket de esa conexión de Query Control
    t_list* queries;                   // queries de este Query Control (t_query*)
} t_conexion_query_control;

extern t_list* CONEXIONES_QUERY_CONTROL;    // Listado de conexiones de Query Control
extern t_list* COLA_READY;                  // Cola de READY (t_query*)
extern t_list* LISTA_EXEC;                  // Lista de EXEC (cada worker define cuántos executes puede haber en esta lista)

extern pthread_mutex_t MUTEX_QUERY_CONTROL;    // Mutex para proteger la lista de conexiones de Query Control
extern pthread_mutex_t MUTEX_READY;            // Mutex para proteger COLA_READY (y LISTA_EXEC cuando se use)
extern int PROXIMO_QUERY_ID;                   // Contador atómico y compartido de IDs de Query
extern e_algoritmo_planificacion ALGORITMO_ACTUAL; // Algoritmo de planificación activo

void inicializar_estado_master(void);
void finalizar_estado_master(void);
void configurar_algoritmo_planificacion(const char* algoritmo_str);

t_query* crear_query(int socket_query_control, const char* path_query, int prioridad);
void encolar_en_ready(t_query* query);                                      // Encola según algoritmo configurado
t_query* obtener_proxima_query_ready(void);                                // Obtiene según algoritmo configurado
int cantidad_en_ready(void);                                                // Cantidad de elementos en READY (útil para logs)
void asociar_query_a_conexion(int socket_query_control, t_query* query);   // Asocia la query al dueño (Query Control) para poder limpiarla/identificarla luego

#endif

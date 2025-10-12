#ifndef MASTER_STATE_H
#define MASTER_STATE_H

#include <commons/collections/list.h>
#include <pthread.h>

typedef enum {
    Q_READY,
    Q_EXEC,
    Q_EXIT
} q_state;

typedef struct {
    int   query_id;
    int   prioridad;
    char* path_query;
    int   qc_socket;
    q_state estado;
} t_query;

// Representa una conexión de Query Control
typedef struct {
    int     qc_socket;      // socket de esa conexión de QC
    t_list* queries;        // queries de este QC (t_query*)
} t_qc_conn;

extern t_list* QC_CONNS;    // Listado de conexiones QC
extern t_list* COLA_READY;  // Cola de READY (t_query*)
extern t_list* LISTA_EXEC;  // Lista de EXEC (cada worker me define cuantos executes puedo tener en esa lista)

extern pthread_mutex_t M_QC;    // Mutex para proteger la lista de conexiones QC
extern pthread_mutex_t M_READY; // Mutex para proteger COLA_READY (y LISTA_EXEC cuando la use)
extern int NEXT_QUERY_ID;       // Contador atómico y compartido de IDs de Query

void master_state_init(void);
void master_state_shutdown(void);

t_query* query_new(int qc_socket, const char* path_query, int prioridad);
void ready_enqueue(t_query* q);                     //Encola en READY (FIFO)
int ready_count(void);                              //Cantidad de elementos en READY (útil para logs)
void qc_attach_query(int qc_socket, t_query* q);    //Asocia la query al dueño (QC) para poder limpiarla/identificarla luego

#endif

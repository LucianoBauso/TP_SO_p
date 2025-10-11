#include "master_state.h"
#include <stdlib.h>

t_list* QC_CONNS    = NULL;
t_list* COLA_READY  = NULL;
t_list* LISTA_EXEC  = NULL;

pthread_mutex_t M_QC    = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t M_READY = PTHREAD_MUTEX_INITIALIZER;

int NEXT_QUERY_ID = 0;

void master_state_init(void) {
    QC_CONNS   = list_create();
    COLA_READY = list_create();
    LISTA_EXEC = list_create();
}

static t_qc_conn* _qc_find_by_socket(int qc_socket) {
    t_qc_conn* found = NULL;
    pthread_mutex_lock(&M_QC);
    for (int i = 0; i < list_size(QC_CONNS); i++) {
        t_qc_conn* it = list_get(QC_CONNS, i);
        if (it->qc_socket == qc_socket) { found = it; break; }
    }
    pthread_mutex_unlock(&M_QC);
    return found;
}

t_query* query_new(int qc_socket, const char* path_query, int prioridad) {
    t_query* q = malloc(sizeof(t_query));
    q->query_id   = __atomic_fetch_add(&NEXT_QUERY_ID, 1, __ATOMIC_SEQ_CST);
    q->prioridad  = prioridad;
    q->path_query = strdup(path_query ? path_query : "");
    q->qc_socket  = qc_socket;
    q->estado     = Q_READY;
    return q;
}

void ready_enqueue(t_query* q) {
    pthread_mutex_lock(&M_READY);
    list_add(COLA_READY, q);
    pthread_mutex_unlock(&M_READY);
}

int ready_count(void) {
    pthread_mutex_lock(&M_READY);
    int n = list_size(COLA_READY);
    pthread_mutex_unlock(&M_READY);
    return n;
}

void qc_attach_query(int qc_socket, t_query* q) {
    pthread_mutex_lock(&M_QC);
    for (int i = 0; i < list_size(QC_CONNS); i++) {
        t_qc_conn* it = list_get(QC_CONNS, i);
        if (it->qc_socket == qc_socket) {
            list_add(it->queries, q);
            break;
        }
    }
    pthread_mutex_unlock(&M_QC);
}

static void _free_query(void* elem) {
    t_query* q = (t_query*) elem;
    if (!q) return;
    free(q->path_query);
    free(q);
}

static void _free_qc_conn(void* elem) {
    t_qc_conn* qc = (t_qc_conn*) elem;
    if (!qc) return;
    if (qc->queries) list_destroy_and_destroy_elements(qc->queries, _free_query);
    free(qc);
}

void master_state_shutdown(void) {
    if (QC_CONNS)   { list_destroy_and_destroy_elements(QC_CONNS, _free_qc_conn); QC_CONNS = NULL; }
    if (COLA_READY) { list_destroy_and_destroy_elements(COLA_READY, _free_query); COLA_READY = NULL; }
    if (LISTA_EXEC) { list_destroy_and_destroy_elements(LISTA_EXEC, _free_query); LISTA_EXEC = NULL; }

    pthread_mutex_destroy(&M_QC);
    pthread_mutex_destroy(&M_READY);
}

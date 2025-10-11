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

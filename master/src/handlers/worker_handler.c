#include "worker_handler.h"
#include "../state/master_state.h"
#include "../core/master.h"
#include "utils/serverUtils.h"
#include <commons/collections/list.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

static t_list *WORKERS = NULL;
static pthread_mutex_t M_WORKERS = PTHREAD_MUTEX_INITIALIZER;

static void _workers_init_once(void)
{
    static int inited = 0;
    if (!inited)
    {
        WORKERS = list_create();
        inited = 1;
    }
}

void registrar_worker(int worker_sock)
{
    _workers_init_once();
    t_worker *w = malloc(sizeof(t_worker));
    w->worker_sock = worker_sock;
    w->ocupado = 0;

    pthread_mutex_lock(&M_WORKERS);
    list_add(WORKERS, w);
    pthread_mutex_unlock(&M_WORKERS);

    if (logger)
        log_info(logger, "## Se conecta un Worker (socket=%d)", worker_sock);
}

void remover_worker(int worker_sock)
{
    pthread_mutex_lock(&M_WORKERS);
    for (int i = 0; i < list_size(WORKERS); i++)
    {
        t_worker *w = list_get(WORKERS, i);
        if (w->worker_sock == worker_sock)
        {
            list_remove(WORKERS, i);
            free(w);
            break;
        }
    }
    pthread_mutex_unlock(&M_WORKERS);

    if (logger)
        log_info(logger, "## Se desconecta un Worker (socket=%d)", worker_sock);
    close(worker_sock);
}

// Aux: devuelve un Worker libre o NULL
static t_worker *_tomar_worker_libre(void)
{
    t_worker *freew = NULL;
    pthread_mutex_lock(&M_WORKERS);
    for (int i = 0; i < list_size(WORKERS); i++)
    {
        t_worker *w = list_get(WORKERS, i);
        if (!w->ocupado)
        {
            freew = w;
            break;
        }
    }
    if (freew)
        freew->ocupado = 1; // lo marcamos ocupado al asignar
    pthread_mutex_unlock(&M_WORKERS);
    return freew;
}

// Aux: saca una query de READY (FIFO) o NULL
static t_query *_ready_pop(void)
{
    t_query *q = NULL;
    pthread_mutex_lock(&M_READY);
    if (list_size(COLA_READY) > 0)
    {
        q = list_remove(COLA_READY, 0);
        if (q)
            q->estado = Q_EXEC;
        // La moveremos a LISTA_EXEC
        if (q)
            list_add(LISTA_EXEC, q);
    }
    pthread_mutex_unlock(&M_READY);
    return q;
}

static int _enviar_asignacion_al_worker(t_worker *w, t_query *q)
{
    // Enviar al Worker: PAQUETE con [query_id, path_query, prioridad]
    t_paquete* paquete = crear_paquete();
    paquete->codigo_operacion = PAQUETE;
    char idbuf[32], priobuf[32];
    snprintf(idbuf, sizeof idbuf, "%d", q->query_id);
    snprintf(priobuf, sizeof priobuf, "%d", q->prioridad);

    agregar_a_paquete(paquete, idbuf, strlen(idbuf) + 1);        // [0] query_id
    agregar_a_paquete(paquete, q->path_query, strlen(q->path_query) + 1); // [1] path_query
    agregar_a_paquete(paquete, priobuf, strlen(priobuf) + 1);    // [2] prioridad

    enviar_paquete(paquete, w->worker_sock);
    eliminar_paquete(paquete);

    return 0; // enviar_paquete no retorna valor, asumimos éxito
}

void planificador_intentar_despachar(void)
{
    // Tomamos una query READY y un worker libre; si cualquiera falta, no hacemos nada.
    t_query *q = _ready_pop();
    if (!q)
        return;

    t_worker *w = _tomar_worker_libre();
    if (!w)
    {
        // No había worker libre → devolvemos la query a READY
        pthread_mutex_lock(&M_READY);
        q->estado = Q_READY;
        list_add(COLA_READY, q);
        pthread_mutex_unlock(&M_READY);
        return;
    }

    log_info(logger, "## Se asigna la Query %d (path=%s, prio=%d) al Worker (socket=%d)",
             q->query_id, q->path_query, q->prioridad, w->worker_sock);

    int rc = _enviar_asignacion_al_worker(w, q);
    if (rc < 0)
    {
        // Falló envío → liberamos worker y finalizamos la query con error (o la devolvemos a READY)
        log_error(logger, "Error enviando Query %d al Worker %d. Se reintenta o se finaliza con error.",
                  q->query_id, w->worker_sock);

        // Devolvemos la query a READY para no perderla (comportamiento simple)
        pthread_mutex_lock(&M_READY);
        q->estado = Q_READY;
        // Sacamos de EXEC si se agregó
        for (int i = 0; i < list_size(LISTA_EXEC); i++)
        {
            if (list_get(LISTA_EXEC, i) == q)
            {
                list_remove(LISTA_EXEC, i);
                break;
            }
        }
        list_add(COLA_READY, q);
        pthread_mutex_unlock(&M_READY);

        // Marcar worker libre
        pthread_mutex_lock(&M_WORKERS);
        w->ocupado = 0;
        pthread_mutex_unlock(&M_WORKERS);
        return;
    }

    // Envío OK: la query queda en EXEC (ya movida) y el worker queda ocupado.
    // En un paso posterior, cuando el Worker envíe “fin” o “lectura”, manejaremos el ida y vuelta.
}

void manejar_worker(int worker_sock)
{
    registrar_worker(worker_sock);

    // Tras registrar, intentamos despachar (por si ya hay READY esperando)
    planificador_intentar_despachar();

    // Por ahora nos quedamos “escuchando” y si el worker se cae, lo removemos.
    // Más adelante, acá procesaremos READ/FIN del worker.
    char buf[1];
    while (1)
    {
        ssize_t r = recv(worker_sock, buf, sizeof(buf), 0);
        if (r <= 0)
        {
            remover_worker(worker_sock);

            // Si un worker se desconecta y había queries en READY, podemos intentar despachar a otro
            planificador_intentar_despachar();
            break;
        }
    }
}

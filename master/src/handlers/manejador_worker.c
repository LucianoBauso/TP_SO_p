#include "manejador_worker.h"
#include "../state/estado_master.h"
#include "../core/master.h"
#include "utils/serverUtils.h"
#include <commons/collections/list.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

static t_list *WORKERS = NULL;
static pthread_mutex_t MUTEX_WORKER = PTHREAD_MUTEX_INITIALIZER;

static void _inicializar_lista_workers(void)
{
    static int inited = 0;
    if (!inited)
    {
        WORKERS = list_create();
        inited = 1;
    }
}

void registrar_worker(int socket_worker)
{
    _inicializar_lista_workers();
    t_worker *w = malloc(sizeof(t_worker));
    w->socket_worker = socket_worker;
    w->ocupado = 0;

    pthread_mutex_lock(&MUTEX_WORKER);
    list_add(WORKERS, w);
    pthread_mutex_unlock(&MUTEX_WORKER);

    if (logger)
        log_info(logger, "## Se conecta un Worker (socket=%d)", socket_worker);
}

void remover_worker(int socket_worker)
{
    pthread_mutex_lock(&MUTEX_WORKER);
    for (int i = 0; i < list_size(WORKERS); i++)
    {
        t_worker *w = list_get(WORKERS, i);
        if (w->socket_worker == socket_worker)
        {
            list_remove(WORKERS, i);
            free(w);
            break;
        }
    }
    pthread_mutex_unlock(&MUTEX_WORKER);

    if (logger)
        log_info(logger, "## Se desconecta un Worker (socket=%d)", socket_worker);
    close(socket_worker);
}

static t_worker *_tomar_worker_libre(void)
{
    t_worker *freew = NULL;
    pthread_mutex_lock(&MUTEX_WORKER);
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
    pthread_mutex_unlock(&MUTEX_WORKER);
    return freew;
}

void planificador_intentar_despachar(void)
{
    // Tomamos una query READY y un worker libre; si cualquiera falta, no hacemos nada.
    t_query *q = obtener_proxima_query_ready();
    if (!q)
        return;

    t_worker *w = _tomar_worker_libre();
    if (!w)
    {
        // No había worker libre → devolvemos la query a READY
        pthread_mutex_lock(&MUTEX_READY);
        q->estado = QUERY_READY;
        list_add(COLA_READY, q);
        pthread_mutex_unlock(&MUTEX_READY);
        return;
    }

    log_info(logger, "## Se asigna la Query %d (path=%s, prio=%d) al Worker (socket=%d)",
             q->query_id, q->path_query, q->prioridad, w->socket_worker);

    int rc = _enviar_asignacion_al_worker(w, q);
    if (rc < 0)
    {
        // Falló envío → liberamos worker y finalizamos la query con error (o la devolvemos a READY)
        log_error(logger, "Error enviando Query %d al Worker %d. Se reintenta o se finaliza con error.",
                  q->query_id, w->socket_worker);

        // Devolvemos la query a READY para no perderla (comportamiento simple)
        pthread_mutex_lock(&MUTEX_READY);
        q->estado = QUERY_READY;
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
        pthread_mutex_unlock(&MUTEX_READY);

        // Marcar worker libre
        pthread_mutex_lock(&MUTEX_WORKER);
        w->ocupado = 0;
        pthread_mutex_unlock(&MUTEX_WORKER);
        return;
    }

    // Envío OK: la query queda en EXEC (ya movida) y el worker queda ocupado.
    // En un paso posterior, cuando el Worker envíe "fin" o "lectura", manejaremos el ida y vuelta.
}

void manejar_worker(int socket_worker)
{
    registrar_worker(socket_worker);

    // Tras registrar, intentamos despachar (por si ya hay READY esperando)
    planificador_intentar_despachar();

    // Por ahora nos quedamos "escuchando" y si el worker se cae, lo removemos.
    // Más adelante, acá procesaremos READ/FIN del worker.
    char buf[1];
    while (1)
    {
        ssize_t r = recv(socket_worker, buf, sizeof(buf), 0);
        if (r <= 0)
        {
            remover_worker(socket_worker);

            // Si un worker se desconecta y había queries en READY, podemos intentar despachar a otro
            planificador_intentar_despachar();
            break;
        }
    }
}

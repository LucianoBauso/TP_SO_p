#define _POSIX_C_SOURCE 200809L
#include "server.h"
#include "ops.h"
#include "logger.h"
#include <commons/log.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int sock;
    ops_ctx_t ctx;
} thread_arg_t;

static void* _thread_main(void* argp) {
    thread_arg_t* a = (thread_arg_t*)argp;

    struct sockaddr_in addr; socklen_t alen = sizeof(addr);
    getpeername(a->sock, (struct sockaddr*)&addr, &alen);
    char ip[64]; inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof ip);
    log_info(storage_log, "##Se conecta un Worker desde %s:%u", ip, ntohs(addr.sin_port));

    ops_handle_connection(a->sock, &a->ctx);

    log_info(storage_log, "##Se desconecta el Worker %s:%u", ip, ntohs(addr.sin_port));
    close(a->sock);
    free(a);
    return NULL;
}

static const storage_cfg_t* g_cfg_compat = NULL;
static const superblock_t*  g_sb_compat  = NULL;

void server_set_context(const storage_cfg_t* cfg, const superblock_t* sb) {
    g_cfg_compat = cfg;
    g_sb_compat  = sb;
}

int crear_server_storage(void) {
    if (!g_cfg_compat || !g_sb_compat) {
        log_error(storage_log, "crear_server_storage(): contexto no seteado. Llamá antes a server_set_context()");
        return -1;
    }
    return server_listen_and_serve(g_cfg_compat, g_sb_compat);
}

int server_listen_and_serve(const storage_cfg_t* cfg, const superblock_t* sb) {
    int srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0) { log_error(storage_log, "socket: %m"); return -1; }

    int yes = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);

    struct sockaddr_in sin = {0};
    sin.sin_family = AF_INET;
    sin.sin_port   = htons((uint16_t)cfg->puerto_escucha);
    sin.sin_addr.s_addr = inet_addr(cfg->ip_escucha ? cfg->ip_escucha : "0.0.0.0");

    if (bind(srv, (struct sockaddr*)&sin, sizeof sin) < 0) { log_error(storage_log, "bind: %m"); close(srv); return -1; }
    if (listen(srv, 64) < 0) { log_error(storage_log, "listen: %m"); close(srv); return -1; }

    log_info(storage_log, "Storage escuchando en %s:%d (BLOCK_SIZE=%u)",
             cfg->ip_escucha ? cfg->ip_escucha : "0.0.0.0",
             cfg->puerto_escucha, sb->block_size_bytes);

    for (;;) {
        int cli = accept(srv, NULL, NULL);
        if (cli < 0) { log_error(storage_log, "accept: %m"); continue; }

        thread_arg_t* a = calloc(1, sizeof *a);
        a->sock = cli;
        a->ctx.cfg = cfg;
        a->ctx.sb  = sb;

        pthread_t th;
        pthread_create(&th, NULL, _thread_main, a);
        pthread_detach(th);
    }
}
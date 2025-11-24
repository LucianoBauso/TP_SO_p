#define _POSIX_C_SOURCE 200809L
#include "ops.h"
#include "logger.h"
#include "protocol.h"

#include <commons/log.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

static void msleep(int ms) {
    if (ms <= 0) return;
    struct timespec ts;
    ts.tv_sec  = ms / 1000;
    ts.tv_nsec = (long)(ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}

static int _send_status(int fd, uint32_t opcode, uint32_t status) {
    uint32_t st = htonl(status);
    return proto_send_frame(fd, opcode, &st, sizeof st);
}

static int _handle_handshake(int fd, const ops_ctx_t* ctx, const void* payload, uint32_t len) {
    log_debug(storage_log, "HANDSHAKE len=%u", len);
    if (len != 4) {
        log_warning(storage_log, "HANDSHAKE payload inválido");
        uint32_t resp[2] = { htonl(1), htonl(0) };
        return proto_send_frame(fd, OP_HANDSHAKE, resp, sizeof resp);
    }
    uint32_t wid_n; memcpy(&wid_n, payload, 4);
    uint32_t worker_id = ntohl(wid_n);
    log_info(storage_log, "Handshake de worker_id=%u OK", worker_id);

    uint32_t resp[2] = { htonl(0), htonl(ctx->sb->block_size_bytes) };
    return proto_send_frame(fd, OP_HANDSHAKE, resp, sizeof resp);
}

static int _handle_get_block_size(int fd, const ops_ctx_t* ctx) {
    log_debug(storage_log, "GET_BLOCK_SIZE");
    uint32_t bs = htonl(ctx->sb->block_size_bytes);
    return proto_send_frame(fd, OP_GET_BLOCK_SIZE, &bs, sizeof bs);
}

static int _handle_read_block(int fd, const ops_ctx_t* ctx, const void* payload, uint32_t len) {
    log_debug(storage_log, "READ_BLOCK len=%u", len);
    if (len != 4) {
        log_warning(storage_log, "READ_BLOCK payload inválido");
        return _send_status(fd, OP_READ_BLOCK, 1);
    }
    msleep(ctx->cfg->retardo_acceso_bloque_ms);
    msleep(ctx->cfg->retardo_operacion_ms);

    uint32_t n = ctx->sb->block_size_bytes;
    void* buf = malloc(n);
    if (!buf) {
        log_error(storage_log, "READ_BLOCK: sin memoria");
        return _send_status(fd, OP_READ_BLOCK, 2);
    }
    memset(buf, 'X', n);
    int rc = proto_send_frame(fd, OP_READ_BLOCK, buf, n);
    free(buf);
    return rc;
}

static int _handle_simple_ok(int fd, uint32_t opcode, const ops_ctx_t* ctx) {
    log_debug(storage_log, "OP %u simple-OK", opcode);
    msleep(ctx->cfg->retardo_operacion_ms);
    return _send_status(fd, opcode, 0);
}

void ops_handle_connection(int client_fd, const ops_ctx_t* ctx) {
    for (;;) {
        uint32_t op=0, len=0;
        void* payload=NULL;

        if (proto_recv_frame(client_fd, &op, &payload, &len)) {
            int e = errno;
            if (payload) free(payload);
            log_debug(storage_log, "recv_frame cortó (errno=%d)", e);
            break; // cliente cerró o error de E/S
        }

        int rc = 0;
        switch (op) {
            case OP_HANDSHAKE:
                rc = _handle_handshake(client_fd, ctx, payload, len);
                break;
            case OP_GET_BLOCK_SIZE:
                rc = _handle_get_block_size(client_fd, ctx);
                break;
            case OP_READ_BLOCK:
                rc = _handle_read_block(client_fd, ctx, payload, len);
                break;
            case OP_CREATE:
            case OP_TRUNCATE:
            case OP_TAG:
            case OP_COMMIT:
            case OP_WRITE_BLOCK:
            case OP_DELETE_TAG:
            case OP_FLUSH:
                rc = _handle_simple_ok(client_fd, op, ctx);
                break;
            default:
                log_warning(storage_log, "Opcode desconocido: %u", op);
                rc = _send_status(client_fd, op, 1);
                break;
        }

        if (payload) free(payload);
        if (rc) { log_debug(storage_log, "send_frame falló (rc=%d)", rc); break; }
    }
}

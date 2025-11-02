#define _POSIX_C_SOURCE 200809L
#include "ops.h"
#include "logger.h"
#include "protocol.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>

static void msleep(int ms) { usleep((useconds_t)ms * 1000); }

static int send_status_ok(int fd, uint32_t opcode_reply) {
    uint32_t status_ok = 0;
    return proto_send_u32(fd, opcode_reply, status_ok);
}

int ops_handle_connection(int sockfd, const ops_ctx_t* ctx) {
    for (;;) {
        uint32_t op, len; void* payload = NULL;
        if (proto_recv_frame(sockfd, &op, &payload, &len)) {
            free(payload);
            break;
        }

        msleep(ctx->cfg->retardo_operacion_ms);

        switch (op) {
            case OP_HANDSHAKE: {
                uint32_t worker_id = 0;
                if (len == 4 && payload) {
                    uint32_t netv; memcpy(&netv, payload, 4);
                    worker_id = ntohl(netv);
                }
                free(payload);

                log_info(storage_log, "##Se conecta el Worker %u (handshake)", worker_id);

                uint32_t resp[2] = { htonl(0), htonl(ctx->sb->block_size_bytes) };
                if (proto_send_frame(sockfd, OP_HANDSHAKE, resp, sizeof(resp))) return -1;
            } break;

            case OP_GET_BLOCK_SIZE: {
                free(payload);
                if (proto_send_u32(sockfd, OP_GET_BLOCK_SIZE, ctx->sb->block_size_bytes)) return -1;
            } break;

            case OP_CREATE:
            case OP_TRUNCATE:
            case OP_TAG:
            case OP_COMMIT:
            case OP_WRITE_BLOCK:
            case OP_DELETE_TAG: {
                free(payload);
                if (send_status_ok(sockfd, op)) return -1;
            } break;

            case OP_READ_BLOCK: {
                free(payload);
                msleep(ctx->cfg->retardo_acceso_bloque_ms);

                uint32_t n = ctx->sb->block_size_bytes;
                void* buf = malloc(n);
                memset(buf, 'X', n);
                if (proto_send_frame(sockfd, OP_READ_BLOCK, buf, n)) { free(buf); return -1; }
                free(buf);
            } break;

            default:
                free(payload);
                return -1;
        }
    }
    return 0;
}
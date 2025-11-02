//Pruebas del Storage

#define _POSIX_C_SOURCE 200809L
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "protocol.h"

enum {
    OP_HANDSHAKE = 1,
    OP_GET_BLOCK_SIZE,
    OP_CREATE,
    OP_TRUNCATE,
    OP_TAG,
    OP_COMMIT,
    OP_WRITE_BLOCK,
    OP_READ_BLOCK,
    OP_DELETE_TAG
};

static uint32_t g_block_size = 0;

int storage_client_connect(const char* host, int port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) { perror("socket"); return -1; }
    struct sockaddr_in sin = {0};
    sin.sin_family = AF_INET;
    sin.sin_port   = htons((uint16_t)port);
    if (inet_pton(AF_INET, host, &sin.sin_addr) != 1) { perror("inet_pton"); close(fd); return -1; }
    if (connect(fd, (struct sockaddr*)&sin, sizeof sin) < 0) { perror("connect"); close(fd); return -1; }
    return fd;
}

int storage_handshake(int fd, uint32_t worker_id, uint32_t* out_block_size) {
    uint32_t wid_n = htonl(worker_id);
    if (proto_send_frame(fd, OP_HANDSHAKE, &wid_n, sizeof wid_n)) return -1;

    uint32_t op, len; void* payload=NULL;
    if (proto_recv_frame(fd, &op, &payload, &len)) return -1;
    if (op != OP_HANDSHAKE || len != 8 || !payload) { free(payload); return -1; }

    uint32_t net_status, net_bs;
    memcpy(&net_status, payload, 4);
    memcpy(&net_bs, (char*)payload+4, 4);
    free(payload);

    if (ntohl(net_status) != 0) return -1;
    g_block_size = ntohl(net_bs);
    if (out_block_size) *out_block_size = g_block_size;
    return 0;
}

int storage_get_block_size(int fd, uint32_t* out_bs) {
    if (proto_send_frame(fd, OP_GET_BLOCK_SIZE, NULL, 0)) return -1;
    uint32_t op,len; void* payload=NULL;
    if (proto_recv_frame(fd, &op, &payload, &len)) return -1;
    if (op != OP_GET_BLOCK_SIZE || len != 4 || !payload) { free(payload); return -1; }
    uint32_t net; memcpy(&net, payload, 4); free(payload);
    *out_bs = ntohl(net);
    return 0;
}

int storage_read_block(int fd, uint32_t logical_index, void** out_buf, uint32_t* out_len) {
    uint32_t idx_n = htonl(logical_index);
    if (proto_send_frame(fd, OP_READ_BLOCK, &idx_n, sizeof idx_n)) return -1;
    uint32_t op, len; void* payload=NULL;
    if (proto_recv_frame(fd, &op, &payload, &len)) return -1;
    if (op != OP_READ_BLOCK || len == 0 || !payload) { free(payload); return -1; }
    *out_buf = payload;
    *out_len = len;
    return 0;
}

int storage_simple_ok(int fd, uint32_t opcode) {
    if (proto_send_frame(fd, opcode, NULL, 0)) return -1;
    uint32_t op, len; void* payload=NULL;
    if (proto_recv_frame(fd, &op, &payload, &len)) return -1;
    if (op != opcode || len != 4 || !payload) { free(payload); return -1; }
    uint32_t net; memcpy(&net, payload, 4); free(payload);
    return (ntohl(net) == 0) ? 0 : -1;
}

int storage_create(int fd, const char* file_tag) {
    if (proto_send_frame(fd, OP_CREATE, file_tag, (uint32_t)strlen(file_tag))) return -1;
    uint32_t op, len; void* payload=NULL;
    if (proto_recv_frame(fd, &op, &payload, &len)) return -1;
    if (op != OP_CREATE || len != 4) { free(payload); return -1; }
    uint32_t net; memcpy(&net, payload, 4); free(payload);
    return (ntohl(net) == 0) ? 0 : -1;
}

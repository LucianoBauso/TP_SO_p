#define _POSIX_C_SOURCE 200809L
#include "protocol.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

static int _write_all(int fd, const void* buf, size_t n) {
    const char* p = buf;
    while (n) {
        ssize_t w = write(fd, p, n);
        if (w <= 0) return -1;
        p += w; n -= (size_t)w;
    }
    return 0;
}
static int _read_all(int fd, void* buf, size_t n) {
    char* p = buf;
    while (n) {
        ssize_t r = read(fd, p, n);
        if (r <= 0) return -1;
        p += r; n -= (size_t)r;
    }
    return 0;
}

int proto_send_frame(int fd, uint32_t opcode, const void* payload, uint32_t length) {
    uint32_t op_n  = htonl(opcode);
    uint32_t len_n = htonl(length);
    if (_write_all(fd, &op_n,  sizeof op_n )) return -1;
    if (_write_all(fd, &len_n, sizeof len_n)) return -1;
    if (length && payload) {
        if (_write_all(fd, payload, length)) return -1;
    }
    return 0;
}

int proto_recv_frame(int fd, uint32_t* opcode, void** payload, uint32_t* length) {
    uint32_t op_n, len_n;
    if (_read_all(fd, &op_n,  sizeof op_n )) return -1;
    if (_read_all(fd, &len_n, sizeof len_n)) return -1;
    *opcode = ntohl(op_n);
    *length = ntohl(len_n);
    if (*length) {
        *payload = malloc(*length);
        if (!*payload) return -1;
        if (_read_all(fd, *payload, *length)) { free(*payload); *payload=NULL; return -1; }
    } else {
        *payload = NULL;
    }
    return 0;
}

int proto_send_u32(int fd, uint32_t opcode, uint32_t value) {
    uint32_t v = htonl(value);
    return proto_send_frame(fd, opcode, &v, sizeof v);
}

int proto_recv_u32_payload(int fd, uint32_t* value_out) {
    uint32_t op; void* payload=NULL; uint32_t len=0;
    if (proto_recv_frame(fd, &op, &payload, &len)) return -1;
    if (len != 4 || !payload) { free(payload); return -1; }
    uint32_t netv; memcpy(&netv, payload, 4); free(payload);
    *value_out = ntohl(netv);
    return 0;
}
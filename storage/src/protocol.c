#define _POSIX_C_SOURCE 200809L
#include "protocol.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

static void sleep_ms(int ms) {
    if (ms <= 0) return;
    struct timespec ts;
    ts.tv_sec  = ms / 1000;
    ts.tv_nsec = (long)(ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}

static int _read_full(int fd, void* buf, size_t n) {
    char* p = (char*)buf;
    while (n) {
        ssize_t r = read(fd, p, n);
        if (r < 0) {
            if (errno == EINTR) continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK) { sleep_ms(1); continue; }
            return -1;
        }
        if (r == 0) return -1; // peer cerró
        p += r; n -= (size_t)r;
    }
    return 0;
}

static int _write_full(int fd, const void* buf, size_t n) {
    const char* p = (const char*)buf;
    while (n) {
        ssize_t w = write(fd, p, n);
        if (w < 0) {
            if (errno == EINTR) continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK) { sleep_ms(1); continue; }
            return -1;
        }
        if (w == 0) continue;
        p += w; n -= (size_t)w;
    }
    return 0;
}

int proto_send_frame(int fd, uint32_t opcode, const void* payload, uint32_t length) {
    uint32_t op_n  = htonl(opcode);
    uint32_t len_n = htonl(length);
    if (_write_full(fd, &op_n,  sizeof op_n )) return -1;
    if (_write_full(fd, &len_n, sizeof len_n)) return -1;
    if (length && payload) {
        if (_write_full(fd, payload, length)) return -1;
    }
    return 0;
}

int proto_recv_frame(int fd, uint32_t* opcode, void** payload, uint32_t* length) {
    uint32_t op_n=0, len_n=0;
    if (_read_full(fd, &op_n,  sizeof op_n )) return -1;
    if (_read_full(fd, &len_n, sizeof len_n)) return -1;

    *opcode = ntohl(op_n);
    *length = ntohl(len_n);

    if (*length) {
        void* p = malloc(*length);
        if (!p) return -1;
        if (_read_full(fd, p, *length)) { free(p); return -1; }
        *payload = p;
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
    uint32_t op=0, len=0; void* payload=NULL;
    if (proto_recv_frame(fd, &op, &payload, &len)) return -1;
    if (len != 4 || !payload) { free(payload); return -1; }
    uint32_t netv; memcpy(&netv, payload, 4); free(payload);
    *value_out = ntohl(netv);
    return 0;
}

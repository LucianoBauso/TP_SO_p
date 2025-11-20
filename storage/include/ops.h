#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "storage_config.h"
#include "fs.h"

typedef struct {
    const storage_cfg_t* cfg;
    const superblock_t*  sb;
} ops_ctx_t;

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

void ops_handle_connection(int client_fd, const ops_ctx_t* ctx);
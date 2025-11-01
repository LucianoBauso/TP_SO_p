#pragma once
#include <stdint.h>
#include "storage_config.h"
#include "fs.h"

typedef enum {
    OP_HANDSHAKE = 1,
    OP_GET_BLOCK_SIZE,
    OP_CREATE,
    OP_TRUNCATE,
    OP_TAG,
    OP_COMMIT,
    OP_WRITE_BLOCK,
    OP_READ_BLOCK,
    OP_DELETE_TAG
} opcode_t;

typedef struct {
    const storage_cfg_t* cfg;
    const superblock_t*  sb;
} ops_ctx_t;

int ops_handle_connection(int sockfd, const ops_ctx_t* ctx);
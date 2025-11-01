#pragma once
#include "storage_config.h"
#include "fs.h"

int server_listen_and_serve(const storage_cfg_t* cfg, const superblock_t* sb);
void server_set_context(const storage_cfg_t* cfg, const superblock_t* sb);
int crear_server_storage(void);
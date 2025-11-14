#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint64_t fs_size_bytes;
    uint32_t block_size_bytes;
    uint32_t blocks_count;
} superblock_t;

bool superblock_load(const char* superblock_cfg_path, superblock_t* out);

char* fs_path_superblock_cfg(const char* mount);
char* fs_path_bitmap(const char* mount);
char* fs_path_blocks_hash_idx(const char* mount);
char* fs_path_physical_blocks_dir(const char* mount);
char* fs_path_files_dir(const char* mount);

bool fs_format_fresh_start(const char* mount, const superblock_t* sb, int* err_out);
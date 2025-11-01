#define _POSIX_C_SOURCE 200809L
#include "fs.h"
#include <commons/string.h>
#include <stdlib.h>

char* fs_path_superblock_cfg(const char* mount) {
    return string_from_format("%s/superblock.config", mount);
}
char* fs_path_bitmap(const char* mount) {
    return string_from_format("%s/bitmap.bin", mount);
}
char* fs_path_blocks_hash_idx(const char* mount) {
    return string_from_format("%s/blocks_hash_index.config", mount);
}
char* fs_path_physical_blocks_dir(const char* mount) {
    return string_from_format("%s/physical_blocks", mount);
}
char* fs_path_files_dir(const char* mount) {
    return string_from_format("%s/files", mount);
}
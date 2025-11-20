#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L
#include "fs.h"
#include "logger.h"
#include <commons/config.h>
#include <commons/string.h>
#include <commons/bitarray.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static int mkdir_p(const char* path, mode_t mode) {
    char* tmp = strdup(path);
    size_t len = strlen(tmp);
    if (len == 0) { free(tmp); return 0; }
    if (tmp[len-1] == '/') tmp[len-1] = 0;

    for (char* p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;
            if (mkdir(tmp, mode) && errno != EEXIST) { free(tmp); return -1; }
            *p = '/';
        }
    }
    if (mkdir(tmp, mode) && errno != EEXIST) { free(tmp); return -1; }
    free(tmp);
    return 0;
}

bool superblock_load(const char* superblock_cfg_path, superblock_t* out) {
    if (!superblock_cfg_path || !out) return false;
    t_config* c = config_create((char*)superblock_cfg_path);
    if (!c) return false;

    uint64_t fs_size  = (uint64_t) config_get_long_value(c, "FS_SIZE");
    uint32_t blk_size = (uint32_t) config_get_int_value(c, "BLOCK_SIZE");
    config_destroy(c);

    if (blk_size == 0) return false;
    out->fs_size_bytes   = fs_size;
    out->block_size_bytes= blk_size;
    out->blocks_count    = (uint32_t)(fs_size / blk_size);
    return out->blocks_count > 0;
}

static bool _write_all(int fd, const void* buf, size_t n) {
    const uint8_t* p = buf;
    while (n) {
        ssize_t w = write(fd, p, n);
        if (w < 0) return false;
        p += (size_t)w; n -= (size_t)w;
    }
    return true;
}

bool fs_format_fresh_start(const char* mount, const superblock_t* sb, int* err_out) {
    if (err_out) *err_out = 0;
    if (!mount || !sb) { if (err_out) *err_out = EINVAL; return false; }

    char* dir_blocks = fs_path_physical_blocks_dir(mount);
    char* dir_files  = fs_path_files_dir(mount);
    if (mkdir_p(dir_blocks, 0755) < 0 || mkdir_p(dir_files, 0755) < 0) {
        if (err_out) *err_out = errno;
        log_error(storage_log, "mkdir_p falló: %s", strerror(errno));
        free(dir_blocks); free(dir_files);
        return false;
    }

    char* path_bitmap = fs_path_bitmap(mount);
    int fd_bm = open(path_bitmap, O_CREAT | O_TRUNC | O_RDWR, 0644);
    if (fd_bm < 0) {
        if (err_out) *err_out = errno;
        log_error(storage_log, "open(bitmap) falló: %s", strerror(errno));
        free(dir_blocks); free(dir_files); free(path_bitmap);
        return false;
    }

    size_t bits     = sb->blocks_count;
    size_t bm_bytes = (bits + 7) / 8;
    if (ftruncate(fd_bm, (off_t)bm_bytes) < 0) {
        if (err_out) *err_out = errno;
        log_error(storage_log, "ftruncate(bitmap) falló: %s", strerror(errno));
        close(fd_bm); free(dir_blocks); free(dir_files); free(path_bitmap);
        return false;
    }

    void* bm_map = mmap(NULL, bm_bytes, PROT_READ | PROT_WRITE, MAP_SHARED, fd_bm, 0);
    if (bm_map == MAP_FAILED) {
        if (err_out) *err_out = errno;
        log_error(storage_log, "mmap(bitmap) falló: %s", strerror(errno));
        close(fd_bm); free(dir_blocks); free(dir_files); free(path_bitmap);
        return false;
    }

    memset(bm_map, 0, bm_bytes);
    t_bitarray* bm = bitarray_create_with_mode((char*)bm_map, bm_bytes, LSB_FIRST);
    if (sb->blocks_count > 0) bitarray_set_bit(bm, 0);
    msync(bm_map, bm_bytes, MS_SYNC);
    bitarray_destroy(bm);
    munmap(bm_map, bm_bytes);
    close(fd_bm);
    free(path_bitmap);

    char* path_idx = fs_path_blocks_hash_idx(mount);
    int fd_idx = open(path_idx, O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (fd_idx >= 0) close(fd_idx);
    free(path_idx);

    for (uint32_t i = 0; i < sb->blocks_count; ++i) {
        char* path_blk = string_from_format("%s/block%04u.dat", dir_blocks, i);
        int fd = open(path_blk, O_CREAT | O_RDWR, 0644);
        if (fd < 0) {
            if (err_out) *err_out = errno;
            log_error(storage_log, "open(%s) falló: %s", path_blk, strerror(errno));
            free(path_blk); free(dir_blocks); free(dir_files);
            return false;
        }
        if (ftruncate(fd, (off_t)sb->block_size_bytes) < 0) {
            if (err_out) *err_out = errno;
            log_error(storage_log, "ftruncate(%s) falló: %s", path_blk, strerror(errno));
            close(fd); free(path_blk); free(dir_blocks); free(dir_files);
            return false;
        }
        if (i == 0) {
            size_t n = sb->block_size_bytes;
            char* zeros = malloc(n);
            memset(zeros, '0', n);
            lseek(fd, 0, SEEK_SET);
            if (!_write_all(fd, zeros, n)) {
                if (err_out) *err_out = EIO;
                log_error(storage_log, "escritura bloque 0 falló");
                free(zeros); close(fd); free(path_blk); free(dir_blocks); free(dir_files);
                return false;
            }
            free(zeros);
        }
        close(fd);
        free(path_blk);
    }

    char* dir_initial  = string_from_format("%s/initial_file/BASE", dir_files);
    char* dir_logical  = string_from_format("%s/logical_blocks", dir_initial);
    if (mkdir_p(dir_initial, 0755) < 0 || mkdir_p(dir_logical, 0755) < 0) {
        if (err_out) *err_out = errno;
        log_error(storage_log, "mkdir initial_file/BASE falló: %s", strerror(errno));
        free(dir_blocks); free(dir_files); free(dir_initial); free(dir_logical);
        return false;
    }

    char* path_meta = string_from_format("%s/metadata.config", dir_initial);
    FILE* fmeta = fopen(path_meta, "w");
    if (!fmeta) {
        if (err_out) *err_out = errno;
        log_error(storage_log, "metadata.config: %s", strerror(errno));
        free(dir_blocks); free(dir_files); free(dir_initial); free(dir_logical); free(path_meta);
        return false;
    }
    fprintf(fmeta, "TAMAÑO=%u\nBLOCKS=[0]\nESTADO=WORK_IN_PROGRESS\n", sb->block_size_bytes);
    fclose(fmeta);

    char* src_phys0 = string_from_format("%s/block%04u.dat", dir_blocks, 0);
    char* dst_log0  = string_from_format("%s/000000.dat", dir_logical);
    unlink(dst_log0);
    if (link(src_phys0, dst_log0) < 0) {
    int in = open(src_phys0, O_RDONLY);
    int out = open(dst_log0, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (in < 0 || out < 0) {
        if (err_out) *err_out = errno;
        log_error(storage_log, "hardlink/copia del bloque 0 falló");
        if (in >= 0) close(in);
        if (out >= 0) close(out);
        free(src_phys0); free(dst_log0); free(dir_blocks); free(dir_files); free(dir_initial); free(dir_logical);
        return false;
    }
    char buf[8192]; ssize_t r;
    while ((r = read(in, buf, sizeof buf)) > 0) {
        if (!_write_all(out, buf, (size_t)r)) { if (err_out) *err_out = EIO; break; }
    }
    close(in);
    close(out);
}

    free(src_phys0); free(dst_log0);
    free(dir_blocks); free(dir_files); free(dir_initial); free(dir_logical);

    log_info(storage_log, "FRESH_START: FS formateado. blocks=%u block_size=%u",
             sb->blocks_count, sb->block_size_bytes);
    return true;
}
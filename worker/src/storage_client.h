#ifndef STORAGE_CLIENT_H_
#define STORAGE_CLIENT_H_

#include <stdint.h>

int storage_client_connect(const char* host, int port);
int storage_handshake(int fd, uint32_t worker_id, uint32_t* out_block_size);
int storage_get_block_size(int fd, uint32_t* out_bs);
int storage_read_block(int fd, uint32_t logical_index, void** out_buf, uint32_t* out_len);
int storage_simple_ok(int fd, uint32_t opcode);
int storage_create(int fd, const char* file_tag);

#endif /* STORAGE_CLIENT_H_ */

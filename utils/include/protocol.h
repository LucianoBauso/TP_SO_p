#pragma once
#include <stdint.h>
#include <stddef.h>

int proto_send_frame(int fd, uint32_t opcode, const void* payload, uint32_t length);
int proto_recv_frame(int fd, uint32_t* opcode, void** payload, uint32_t* length);

int proto_send_u32(int fd, uint32_t opcode, uint32_t value);
int proto_recv_u32_payload(int fd, uint32_t* value_out);
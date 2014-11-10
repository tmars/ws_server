#ifndef FRAME_H_
#define FRAME_H_

#include <sys/types.h>  // size_t

enum OPCODE {
    OPCODE_CONT = 0,
    OPCODE_TEXT = 1,
    OPCODE_BINARY = 2,
    OPCODE_CLOSE = 8,
    OPCODE_PING = 9,
    OPCODE_PONG = 10,
};

struct frame {
    char *payload;
    size_t payload_size;

    char *data;
    size_t size;

    char opcode;
};

struct frame *
frame_parse(const char *buffer, size_t size);

struct frame *
frame_create(const char *payload, size_t size, char opcode);

void
frame_free(struct frame *f);

#endif  // FRAME_H_

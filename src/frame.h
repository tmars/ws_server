#ifndef FRAME_H
#define FRAME_H

#include <sys/types.h> // size_t

struct frame {
	char *payload;
	size_t payload_size;

	char *data;
	size_t size;
};

struct frame *
frame_parse(const char *buffer, int size);

struct frame *
frame_init(const char *payload, int size);

void
frame_free(struct frame *f);

#endif // FRAME_H
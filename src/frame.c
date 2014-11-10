#include "frame.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>

#define ws_ntohl64(p) \
    ((((uint64_t)((p)[0])) <<  0) + (((uint64_t)((p)[1])) <<  8) +\
     (((uint64_t)((p)[2])) << 16) + (((uint64_t)((p)[3])) << 24) +\
     (((uint64_t)((p)[4])) << 32) + (((uint64_t)((p)[5])) << 40) +\
     (((uint64_t)((p)[6])) << 48) + (((uint64_t)((p)[7])) << 56))

#define ws_htonl64(p) {\
    (char)(((p & ((uint64_t)0xff <<  0)) >>  0) & 0xff), (char)(((p & ((uint64_t)0xff <<  8)) >>  8) & 0xff), \
    (char)(((p & ((uint64_t)0xff << 16)) >> 16) & 0xff), (char)(((p & ((uint64_t)0xff << 24)) >> 24) & 0xff), \
    (char)(((p & ((uint64_t)0xff << 32)) >> 32) & 0xff), (char)(((p & ((uint64_t)0xff << 40)) >> 40) & 0xff), \
    (char)(((p & ((uint64_t)0xff << 48)) >> 48) & 0xff), (char)(((p & ((uint64_t)0xff << 56)) >> 56) & 0xff) }

struct frame *
frame_parse(const char *buffer, size_t size)
{
    int has_mask;
    uint64_t len;
    const char *p;  // указатель на payload
    unsigned char mask[4];

    p = buffer;

    // Проверка полного фрейма
    if (size < 8) {
        return NULL;
    }

    // Проверка инициализации буфера
    if (buffer == NULL) {
        return NULL;
    }

    // 8 бит
    has_mask = buffer[1] & 0x80 ? 1:0;

    // Извлечение размера payload
    len = buffer[1] & 0x7f;  // убираем 8 бит
    if (len <= 125) {
        p = buffer + 2;
    } else if (len == 126) {
        // Размер payload 16-ое число
        uint16_t size16;
        memcpy(&size16, buffer + 2, sizeof(uint16_t));
        len = ntohs(size16);
        p = buffer + 4;
    } else if (len == 127) {
        len = ws_ntohl64(buffer + 2);
        p = buffer + 10;
    }

    // данные начинаются сразу после макси
    if (has_mask) {
        memcpy(&mask, p, sizeof(mask));
        p += 4;
    }

    // Неостаточно данных
    if (len > size - (p - buffer)) {
        return NULL;
    }


    if (buffer[0] & 0x80) {  // FIN bit set
        struct frame *f = calloc(1, sizeof(struct frame));

        f->opcode = buffer[0] & 0x0F;
        f->size = len + (p - buffer);
        f->data = malloc(f->size);
        memcpy(f->data, buffer, f->size);

        if (f->opcode == OPCODE_TEXT) {  // добавляем символ '\0'
            f->payload_size = len+1;
            f->payload = malloc(f->payload_size);
            memcpy(f->payload, p, f->payload_size);
            f->payload[len] = '\0';
        } else {
            f->payload_size = len;
            f->payload = malloc(f->payload_size);
            memcpy(f->payload, p, f->payload_size);
        }

        size_t i;
        for (i = 0; i < len && mask; ++i) {
            f->payload[i] = (unsigned char)f->payload[i] ^ mask[i%4];
        }

        return f;
    }

    return NULL;
}

struct frame *
frame_create(const char *payload, size_t size, char opcode)
{
    size_t header_size = 0;
    struct frame *f = calloc(1, sizeof(struct frame));

    if (payload == NULL && size != 0) {
        return NULL;
    }

    f->data = malloc(size + 8);  // фрейм с учетом header
    f->payload = malloc(size);

    f->data[0] = 0x80 | opcode;  // установка флага FIN и OPCODE
    if (size <= 125) {
        f->data[1] = size;
        header_size = 2;
    } else if (size > 125 && size <= 65536) {
        f->data[1] = 126;
        header_size = 4;

        uint16_t size16 = htons(size);
        memcpy(f->data + 2, &size16, 2);
    } else if (size > 65536) {
        f->data[1] = 127;
        header_size = 10;

        char size64[8] = ws_htonl64(size);
        memcpy(f->data + 2, size64, 8);
    }

    f->payload_size = size;
    memcpy(f->payload, payload, size);

    f->size = size + header_size;
    memcpy(f->data + header_size, payload, size);

    return f;
}

void
frame_free(struct frame *f)
{
    free(f->data);
    free(f->payload);
    free(f);
}

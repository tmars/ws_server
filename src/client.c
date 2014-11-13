#include "client.h"
#include "websocket.h"
#include "http.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

struct client*
client_new(int sock)
{
    struct client *c = calloc(1, sizeof(struct client));

    // Делаем сокет не блокирующим
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    c->sock = sock;

    c->on_text_frame = NULL;
    c->on_bin_frame = NULL;
    c->on_ping = NULL;
    c->on_close = NULL;

    return c;
}

void
client_remove_data(struct client *c)
{
    free(c->buffer);
    c->buffer = NULL;
    c->size = 0;
}

int
client_read(struct client *c)
{
    char buffer[4096];
    int n;

    n = read(c->sock, buffer, sizeof(buffer));
    if (n <= 0) {
        return n;
    }

    c->buffer = realloc(c->buffer, c->size + n);
    if (c->buffer == NULL) {
        return -1;
    }
    memcpy(c->buffer + c->size, buffer, n);
    c->size += n;

    return n;
}

int
client_write(struct client *c, char *data, size_t size)
{
    int n;

    n = write(c->sock, data, size);
    if (n <= 0) {
        return n;
    }

    return n;
}

struct frame *
client_receive(struct client *c)
{
    size_t size;

    // Парсим кадр
    struct frame *f = frame_parse(c->buffer, c->size);
    if (f == NULL) {
        return NULL;
    }

    // Освобождаем распарсенные данные
    size = c->size - f->size;
    char *buffer = malloc(size);
    memcpy(buffer, c->buffer + f->size, size);
    client_remove_data(c);
    c->buffer = buffer;
    c->size = size;

    return f;
}

int
client_send(struct client *c, struct frame *f)
{
    if (f == NULL) {
        return -1;
    }

    return client_write(c, f->data, f->size);
}

void
client_work(struct client *c)
{
    int n;

    // Считываем буфер
    do {
        n = client_read(c);
    } while (n <= 0);

    struct http_response *r = get_handshake_response(c->buffer);
    client_remove_data(c);
    if (r == NULL) {
        perror("Error handshake");
        exit(1);
    }

    // Отправляем ответ
    write(c->sock, r->out, r->out_size);
    http_response_free(r);

    while (1) {
        client_read(c);
        struct frame *f = client_receive(c);
        if (f == NULL) {
            continue;
        }

        if (f->opcode == OPCODE_PING) {
            // Пинг-понг
            struct frame *a = frame_create(NULL, 0, OPCODE_PONG);
            client_send(c, a);
            frame_free(a);

            if (c->on_ping != NULL) {
                (*c->on_ping)(c);
            }
        } else if (f->opcode == OPCODE_CLOSE) {
            // Закрытие соединения
            if (c->on_close != NULL) {
                (*c->on_close)(c);
            }
            break;
        } else if (f->opcode == OPCODE_TEXT) {
            // Текстовое сообщение
            if (c->on_text_frame != NULL) {
                (*c->on_text_frame)(c, f->payload, f->payload_size);
            }
        } else if (f->opcode == OPCODE_BINARY) {
            // Бинарное сообщение
            if (c->on_bin_frame != NULL) {
                (*c->on_bin_frame)(c, f->payload, f->payload_size);
            }
        }

        frame_free(f);
    }

    exit(0);
}

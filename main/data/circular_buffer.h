//
// Created by matteo on 4/25/19.
//

#ifndef TCP_SERVER_CIRCULAR_BUFFER_H
#define TCP_SERVER_CIRCULAR_BUFFER_H

#include <stdint.h>

#define CIRCULAR_BUFFER_SIZE 64

typedef struct {
    uint32_t buffer[CIRCULAR_BUFFER_SIZE];
    unsigned char head;
    unsigned char tail;
    int full;
} circular_buffer;

int circular_buf_empty(circular_buffer *cbuf);

void circular_buf_init(circular_buffer *cb);

unsigned char circular_buf_size(circular_buffer *cbuf);

int circular_buf_get(circular_buffer *cbuf, uint32_t *data);

void circular_buf_put(circular_buffer *cbuf, uint32_t data);

#endif //TCP_SERVER_CIRCULAR_BUFFER_H

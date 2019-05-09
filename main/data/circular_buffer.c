//
// Created by matteo on 4/23/19.
//

#include "circular_buffer.h"

void circular_buf_init(circular_buffer *cb) {
    cb->head = 0;
    cb->tail = 0;
    cb->full = 0;
}

int circular_buf_empty(circular_buffer *cbuf) {
    return (!cbuf->full && (cbuf->head == cbuf->tail));
}

unsigned char circular_buf_size(circular_buffer *cbuf) {
    if(!cbuf->full) {
        return cbuf->tail - cbuf->head;
    }

    return CIRCULAR_BUFFER_SIZE;
}

static void retreat_pointer(circular_buffer *cbuf) {
    cbuf->full = 0;
    cbuf->tail = (unsigned char) ((cbuf->tail + 1) % CIRCULAR_BUFFER_SIZE);
}

int circular_buf_get(circular_buffer *cbuf, uint32_t *data) {
    int r = -1;

    if(!circular_buf_empty(cbuf)) {
        *data = cbuf->buffer[cbuf->tail];
        retreat_pointer(cbuf);

        r = 0;
    }

    return r;
}

static void advance_pointer(circular_buffer *cbuf) {
    if(cbuf->full) {
        cbuf->tail = (unsigned char) ((cbuf->tail + 1) % CIRCULAR_BUFFER_SIZE);
    }

    cbuf->head = (unsigned char) ((cbuf->head + 1) % CIRCULAR_BUFFER_SIZE);
    cbuf->full = (cbuf->head == cbuf->tail);
}

void circular_buf_put(circular_buffer *cbuf, uint32_t data) {
    cbuf->buffer[cbuf->head] = data;

    advance_pointer(cbuf);
}
//
// Created by matteo on 4/23/19.
//

#ifndef TCP_SERVER_TCP_PROTOCOL_H
#define TCP_SERVER_TCP_PROTOCOL_H

#include "data/circular_buffer.h"
#include "freertos/task.h"

typedef struct {
    unsigned char id;
    unsigned char len;
    unsigned char *data;
} packet;

void init_tcp_protocol();

void tcp_server_task(void *args);

void connection_thread(void * p);

#endif //TCP_SERVER_TCP_PROTOCOL_H


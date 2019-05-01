//
// Created by matteo on 4/23/19.
//

#ifndef TCP_SERVER_TCP_PROTOCOL_H
#define TCP_SERVER_TCP_PROTOCOL_H

#include "data/circular_buffer.h"
#include "freertos/task.h"
#include <stdbool.h>

typedef struct {
    bool received;
    unsigned short expectedId;
} keep_alive_t;

void udp_server_task(void *args);

#endif //TCP_SERVER_TCP_PROTOCOL_H


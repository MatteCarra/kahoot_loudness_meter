//
// Created by matteo on 4/23/19.
//

#ifndef TCP_SERVER_SENSOR_THREAD_H
#define TCP_SERVER_SENSOR_THREAD_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "data/circular_buffer.h"

typedef struct {
    circular_buffer * buffer;
    TaskHandle_t taskToNotify;
} sensor_thread_args;

void sensor_thread(void * pvParameters);

#endif //TCP_SERVER_SENSOR_THREAD_H


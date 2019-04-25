//
// Created by matteo on 4/23/19.
//
#include "sensor_thread.h"
#include <string.h>
#include "esp_log.h"

static const char *TAG = "sensor_thread";

void sensor_thread(void *pvParameters) {
    ESP_LOGI(TAG, "Sensor thread started");

    sensor_thread_args *args = (sensor_thread_args *) pvParameters;
    circular_buffer *buffer = args->buffer;
    TaskHandle_t taskToNotify = args->taskToNotify;
    free(args);

    while(1) {
        circular_buf_put(buffer, 25);
        xTaskNotifyGive(taskToNotify);

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
//
// Created by matteo on 4/23/19.
//
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_log.h"

#include "lwip/sockets.h"

#include "tcp_protocol.h"
#include "socket_utils.h"
#include "sensor/sensor_thread.h"

#define PORT CONFIG_EXAMPLE_PORT
#define MAX_SOCKETS 64

static const char *TAG = "tcp_protcol";
static int sockets[MAX_SOCKETS];

void init_sockets() {
    for(int i = 0; i < MAX_SOCKETS; i++) {
        sockets[i] = -1;
    }
}

int add_socket(int socket) {
    int i = 0;
    while(sockets[i] != -1) {
        i++;

        if(i == MAX_SOCKETS)
            return 0; //Can't accept new socket
    }

    sockets[i] = socket;

    return 1;
}

void destroy_socket(int socket) {
    for(int i = 0; i < MAX_SOCKETS; i++) {
        if(sockets[i] == socket) {
            sockets[i] = -1;
        }
    }

    close(socket);
}

void data_sender_thread(void *pvParameters) {
    circular_buffer *buffer = (circular_buffer *) pvParameters;

    ESP_LOGI(TAG, "Data sender thread started");

    unsigned char buff[3];
    unsigned short data;

    int sock_n;
    int socket;
    int ulEventsToProcess;
    for(;;) {
        ulEventsToProcess = ulTaskNotifyTake( pdTRUE, portMAX_DELAY);

        while(ulEventsToProcess > 0 && circular_buf_get(buffer, &data) != -1) {
            buff[0] = 2; //packet id

            //data
            buff[1] = (unsigned char) ((data >> 8) & 0xFF);
            buff[2] = (unsigned char) (data & 0xFF);

            for(sock_n = 0; sock_n < MAX_SOCKETS; sock_n++) {
                socket = sockets[sock_n];
                if(socket != -1) {
                    int err = send(socket, buff, 3, 0);

                    if (err < 0) {
                        ESP_LOGE(TAG, "Error occured during sending: errno %d", errno);
                        destroy_socket(socket);
                        break;
                    }
                }
            }

            ulEventsToProcess--;
        }
    }
}



void tcp_server_task(void *pvParameters) {
    circular_buffer *buffer = (circular_buffer *) pvParameters;

    init_sockets();

    char addr_str[128];
    int addr_family;
    int ip_protocol;

    while (1) {
        struct sockaddr_in destAddr;
        destAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        destAddr.sin_family = AF_INET;
        destAddr.sin_port = htons(PORT);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;
        inet_ntoa_r(destAddr.sin_addr, addr_str, sizeof(addr_str) - 1);


        int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
        if (listen_sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket created");

        int err = bind(listen_sock, (struct sockaddr *)&destAddr, sizeof(destAddr));
        if (err != 0) {
            ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket binded");

        err = listen(listen_sock, 1);
        if (err != 0) {
            ESP_LOGE(TAG, "Error occured during listen: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket listening");

        TaskHandle_t dataSenderThread;
        xTaskCreate(data_sender_thread, "data_sender_thread", 2048, buffer, 5, &dataSenderThread);

        sensor_thread_args *sensorThreadArgs = malloc(sizeof(sensorThreadArgs));
        sensorThreadArgs->buffer = buffer;
        sensorThreadArgs->taskToNotify = dataSenderThread;
        xTaskCreate(sensor_thread, "sensor_thread", 2048, sensorThreadArgs, 5, NULL);

        struct sockaddr_in6 sourceAddr; // Large enough for both IPv4 or IPv6
        uint addrLen = sizeof(sourceAddr);

        while(1) {
            int sock = accept(listen_sock, (struct sockaddr *)&sourceAddr, &addrLen);

            if (sock < 0) {
                ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
                break;
            }

            if(add_socket(sock)) {
                ESP_LOGI(TAG, "Socket accepted");

                //TODO
            } else {
                ESP_LOGE(TAG, "Unable to accept connection: not enough space");
                close(sock);
            }
        }
    }

    vTaskDelete(NULL);
}
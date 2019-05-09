//
// Created by matteo on 4/23/19.
//
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_log.h"

#include "lwip/sockets.h"

#include "udp_protocol.h"
#include "sensor/sensor_thread.h"
#include <stdatomic.h>

#define PORT 3333
#define MAX_SOCKETS 64
#define BUF_SIZE 256

typedef struct {
    struct sockaddr_in socketAddr;
    atomic_bool free;
} client;

static const char *TAG = "udp_protcol";
static client sockets[MAX_SOCKETS];
static int serverSocket;
static time_t keep_alive_per_socket[MAX_SOCKETS];
static atomic_int free_sockets;
static SemaphoreHandle_t recovery_add_socket_mutex;

void init_sockets() {
    srand(time(NULL));

    atomic_init(&free_sockets, MAX_SOCKETS);

    for(int i = 0; i < MAX_SOCKETS; i++) {
        atomic_init(&sockets[i].free, true);
        keep_alive_per_socket[i] = -1;
    }

    recovery_add_socket_mutex = xSemaphoreCreateMutex();
}

bool sockaddr_cmp(struct sockaddr_in *addr1, struct sockaddr_in *addr2) {
    return (addr1->sin_addr.s_addr == addr2->sin_addr.s_addr) && (addr1->sin_port == addr2->sin_port);
}

void on_keep_alive_receive(struct sockaddr_in *addr) {
    int i = 0;
    client * currentClient;

    for(i = 0; i < MAX_SOCKETS; i++) {
        currentClient  = &sockets[i];

        if(atomic_load(&currentClient->free) == false && sockaddr_cmp(addr, &currentClient->socketAddr)) {
            keep_alive_per_socket[i] = time(NULL);
            break;
        }
    }
}

int send_packet(struct sockaddr_in *addr, char * buffer, int len) {
    int res = sendto(serverSocket, buffer, len, 0, (struct sockaddr *) addr, sizeof(struct sockaddr_in));

    if(res == ENOMEM) {
        vTaskDelay(1);
        return send_packet(addr, buffer, len);
    }

    return res;
}

int recovery_add_socket(struct sockaddr_in * socket) {
    time_t now = time(NULL);
    time_t currentTime;
    client * client;

    for(int i = 0; i < MAX_SOCKETS; i++) {
        xSemaphoreTake( recovery_add_socket_mutex, portMAX_DELAY ); //I could very well ignore race conditions because there are not tremendous side effects (like desync of free_sockets) and it's always called in the same thread, but whatever...

        currentTime = keep_alive_per_socket[i];
        client = &sockets[i];

        if(atomic_load(&client->free) == false && currentTime != -1 && now - currentTime > 5) { //check if any client has exceeded the timeout
            keep_alive_per_socket[i] = time(NULL);
            xSemaphoreGive(recovery_add_socket_mutex); //release the mutex after settings keep_alive to now => it won't match the condition again in a concurrent scenario

            send_packet(&client->socketAddr, (char[]) { 2 }, 1); //notify disconnection packet

            memcpy(client, socket, sizeof(struct sockaddr_in));

            return i;
        } else {
            xSemaphoreGive(recovery_add_socket_mutex);
        }
    }

    return -1;
}

int add_socket(struct sockaddr_in * socket) {
    int free_sockets_value = atomic_load(&free_sockets);
    if(free_sockets_value == 0) { //if free sockets is = 0 => try to kick a client that didn't send a keep alive in the last 5 seconds
        return recovery_add_socket(socket);
    }

    bool trueFlag = true;
    client * client;

    for(int i = 0; i < MAX_SOCKETS; i++) {
        client = &sockets[i];

        if(atomic_compare_exchange_weak(&client->free, &trueFlag, false)) { //atomically check if client->free is true and replace it with false
            atomic_fetch_sub(&free_sockets, 1); //decrement free sockets by one
            memcpy(client, socket, sizeof(struct sockaddr_in));
            keep_alive_per_socket[i] = time(NULL);
            return i;
        }

        trueFlag = true;
    }

    return recovery_add_socket(socket);
}

void destroy_socket(struct sockaddr_in * socket) {
    client *client;
    bool falseFlag = false;
    for(int i = 0; i < MAX_SOCKETS; i++) {
        client = &sockets[i];

        if(!atomic_load(&client->free) && sockaddr_cmp(&client->socketAddr, socket) && atomic_compare_exchange_weak(&client->free, &falseFlag, true)) { //atomically check if client->free is false and replace it with true
            atomic_fetch_add(&free_sockets, 1);
            break;
        }

        falseFlag = false;
    }
}

void disconnect_socket(struct sockaddr_in * socket) {
    send_packet(socket, (char[]) { 2 }, 1); //notify disconnection packet

    destroy_socket(socket);
}

void data_sender_thread(void *pvParameters) {
    circular_buffer *buffer = (circular_buffer *) pvParameters;

    ESP_LOGI(TAG, "Data sender thread started");

    char keepAlivePacketBuff[5];
    uint32_t data;

    int sock_n;
    client *socket;
    int ulEventsToProcess;
    for(;;) {
        ulEventsToProcess = ulTaskNotifyTake( pdTRUE, portMAX_DELAY);

        while(ulEventsToProcess > 0 && circular_buf_get(buffer, &data) != -1) {
            keepAlivePacketBuff[0] = 0; //packet id

            //data
            keepAlivePacketBuff[1] = (data >> 24) & 0xFF;
            keepAlivePacketBuff[2] = (data >> 16) & 0xFF;
            keepAlivePacketBuff[3] = (data >> 8) & 0xFF;
            keepAlivePacketBuff[4] = data & 0xFF;

            for(sock_n = 0; sock_n < MAX_SOCKETS; sock_n++) {
                socket = &sockets[sock_n];
                if(!atomic_load(&socket->free)) {
                    int err = send_packet(&socket->socketAddr, keepAlivePacketBuff, 5);

                    if (err < 0) {
                        ESP_LOGE(TAG, "Error occurred during sending: err %s", strerror(errno));
                        destroy_socket(&socket->socketAddr);
                        break;
                    }
                }
            }

            ulEventsToProcess--;
        }
    }
}

void handlePacket(const char * buffer, int len, struct sockaddr_in * addr) {
    unsigned char packetId = buffer[0];

    switch (packetId) {
        case 0: //connect
            if(add_socket(addr) == -1) {
                send_packet(addr, (char []) { 2 }, 1); //notify disconnection packet
            }
            break;
        case 1: //keep alive
            on_keep_alive_receive(addr);
            break;
        case 2: //disconnect
            destroy_socket(addr);
            break;
    }
}

void udp_server_task(void *pvParameters) {
    circular_buffer *buffer = (circular_buffer *) pvParameters;

    char rx_buffer[BUF_SIZE];
    int len;

    //server socket addr
    struct sockaddr_in destAddr;
    int addr_family;
    int ip_protocol;

    //client socket address
    struct sockaddr_in source_addr;
    socklen_t socklen = sizeof(source_addr);

    init_sockets();

    destAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(PORT);
    addr_family = AF_INET;
    ip_protocol = IPPROTO_IP;

    serverSocket = socket(addr_family, SOCK_DGRAM, ip_protocol);
    if (serverSocket < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        return;
    }
    ESP_LOGI(TAG, "Socket created");

    int err = bind(serverSocket, (struct sockaddr *) &destAddr, sizeof(destAddr));
    if (err < 0) {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        return;
    }
    ESP_LOGI(TAG, "Socket bound, port %d", PORT);

    TaskHandle_t dataSenderThread;
    xTaskCreate(data_sender_thread, "data_sender_thread", 4096, buffer, 5, &dataSenderThread);

    sensor_thread_args *sensorThreadArgs = malloc(sizeof(sensor_thread_args));
    sensorThreadArgs->buffer = buffer;
    sensorThreadArgs->taskToNotify = dataSenderThread;
    xTaskCreate(sensor_thread, "sensor_thread", 4096, sensorThreadArgs, 5, NULL);

    while(1) {
        len = recvfrom(serverSocket, rx_buffer, sizeof(rx_buffer), 0, (struct sockaddr *)&source_addr, &socklen);

        if(len < 0) {
            ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
            break;
        } else if(len == 0) {
            vTaskDelay(pdMS_TO_TICKS(50));
            continue;
        }

        handlePacket(rx_buffer, len, &source_addr);
    }

    vTaskDelete(NULL);
}
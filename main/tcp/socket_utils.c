//
// Created by matteo on 4/24/19.
//

#include "socket_utils.h"
#include <string.h>
#include "esp_wifi.h"

int readFully(int socket, unsigned int bytesToRead, void * buffer) {
    int bytesRead = 0;
    int result;
    while (bytesRead < bytesToRead) {
        result = read(socket, buffer + bytesRead, bytesToRead - bytesRead);

        if (result < 1 ) {
            return 0;
        }

        bytesRead += result;
    }
    return 1;
}
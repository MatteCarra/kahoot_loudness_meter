//
// Created by matteo on 4/24/19.
//

#include "socket_utils.h"
#include <string.h>
#include "esp_wifi.h"

int readByte(int socket, unsigned char * byte) {
    if(read(socket, &byte, 1) < 1)
        return 0;

    return 1;
}

int readUShort(int socket, unsigned short * data) {
    unsigned char bytes[2];
    if(readFully(socket, 2, bytes)) {
        *data = (bytes[0] << 8) | bytes[1];
        return 1;
    }
    return 0;
}

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
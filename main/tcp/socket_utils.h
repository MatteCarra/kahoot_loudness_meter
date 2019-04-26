//
// Created by matteo on 4/24/19.
//

#ifndef TCP_SERVER_SOCKET_UTILS_H
#define TCP_SERVER_SOCKET_UTILS_H

int readFully(int socket, unsigned int bytesToRead, void * buffer);

int readByte(int socket, unsigned char * byte);

int readUShort(int socket, unsigned short *data);
#endif //TCP_SERVER_SOCKET_UTILS_H

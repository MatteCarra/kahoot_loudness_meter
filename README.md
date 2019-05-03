# Kahoot loudness meter

This program measures sound and sends to 'connected' UDP clients how loud the sound is in the range [0, 4096).

The server accepts UDP packets on port 3333.
Clients have to send a 'connection packet' in order to be added to the 'connected client list'.

The server accepts a maximum number of 64 clients in this list.
Once the connected list is full new clients are accepted if a client in the list has not sent a keep alive in the past 5 seconds.
In this case the idle client is 'disconnected' and the new client is accommodated.

## Protocol
Packets are prefixed with the packet id (1 byte).
* Packet ids:
    * Server-side:
        * Data packet:
            * Id: 0
            * Data:
                1. Unsigned-Short (2 bytes big-endian): It contains an unsigned short with the sound measurement
        * Disconnect packet:
            * Description: sent by the server when a client is removed from the connected clients list
            * Id: 2
            * Empty data
    * Client-side:
        * Connect packet:
            * Description: It asks the server to add the client address to the connected list to receive data packets
            * Id: 0
            * Empty data
        * Keep alive:
            * Description: It tells the server that the client is still alive
            * Id: 1
            * Empty data
        * Disconnect packet:
            * Description: it tells the server to remove the client from the connected list
            * Empty data

## Hardware Required

This example can be run on any commonly available ESP32 development board: https://www.espressif.com/en/products/hardware/esp32/overview

## Configure the project

```
make menuconfig
```

Set following parameter under Component Config -> WI-FI:

* Set `Max number of WiFi dynamic TX buffers` to 128.

Set following parameters under KahootLoudnessMeter Configuration Options:

* Set `WiFi SSID` of the Router (Access-Point).

* Set `WiFi Password` of the Router (Access-Point).

## Build and Flash

Build the project and flash it to the board, then run monitor tool to view serial output:

```
make -j4 flash monitor
```

(To exit the serial monitor, type ``Ctrl-]``.)
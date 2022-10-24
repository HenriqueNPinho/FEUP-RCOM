// Application layer protocol header.
// NOTE: This file must not be changed.

#ifndef _APPLICATION_LAYER_H_
#define _APPLICATION_LAYER_H_
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "../include/link_layer.h"


void createLinkLayer(int fd, const char* serialPort, LinkLayerRole role, int baudRate, int nRetransmissions, int timeout, int packetSize);
int readFileInformation(const char* fileName);
int sendControlPacket(unsigned char controlByte);
int sendDataPacket();
int sendFile(const char* filename);
// Application layer main function.
// Arguments:
//   serialPort: Serial port name (e.g., /dev/ttyS0).
//   role: Application role {"tx", "rx"}.
//   baudrate: Baudrate of the serial port.
//   nTries: Maximum number of frame retries.
//   timeout: Frame timeout.
//   filename: Name of the file to send / receive.
void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename, int packetSize);
// Arguments:
//  controlByte: control byte for START and END.
//  packet: buffer to use.
//  filename: file name to compare.
int readControlPacket(unsigned char controlByte, unsigned char* packet, const char* filename);

int processDataPacket(unsigned char* buffer);
// Arguments:
//  fd: Connection Port.
//  filename: file name for comparisons.
int receiveFile(const char* filename);

#endif // _APPLICATION_LAYER_H_

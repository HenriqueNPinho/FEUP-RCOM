// Application layer protocol header.
// NOTE: This file must not be changed.

#ifndef _APPLICATION_LAYER_H_
#define _APPLICATION_LAYER_H_
#include "../include/link_layer.h"


void createLinkLayer(const char* serialPort, LinkLayerRole role, int baudRate, int nRetransmissions, int timeout);
int readFileInformation(const char* fileName);
int sendControlPacket(unsigned char controlByte);
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
                      int nTries, int timeout, const char *filename);
// Arguments:
//  controlByte: control byte for START and END.
//  packet: buffer to use.
//  filename: file name to compare.
int readControlPacket(unsigned char controlByte, unsigned char* packet, const char* filename);

// Arguments:
//  fd: Connection Port.
//  filename: file name for comparisons.
int receiveFile(int fd, const char* filename);

#endif // _APPLICATION_LAYER_H_

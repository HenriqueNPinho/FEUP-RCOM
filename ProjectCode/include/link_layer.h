// Link layer header.
// NOTE: This file must not be changed.

#ifndef _LINK_LAYER_H_
#define _LINK_LAYER_H_
#include "utils.h"
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

typedef enum
{
    LlTx,
    LlRx,
} LinkLayerRole;

typedef struct
{
    char serialPort[50];
    int fdPort;
    LinkLayerRole role;
    int baudRate;
    int nRetransmissions;
    int timeout;
    int packetSize;
} LinkLayer;

typedef struct 
{
    const char* fileName;
    int fileSize;
    int fdFile;
    
} ControlPacketInformation;

// SIZE of maximum acceptable payload.
// Maximum number of bytes that application layer should send to link layer
#define MAX_PAYLOAD_SIZE 1000

// MISC
#define FALSE 0
#define TRUE 1

void alarmHandler(int signal);
void startAlarm();
void disableAlarm();

int ControlByteCheck(unsigned char b);
void SMresponse(enum state *currState, unsigned char b, unsigned char* controlb);
void readReceiverResponse(int fd);
void readTransmitterResponse(int fd);
// Open a connection using the "port" parameters defined in struct linkLayer.
// Return "1" on success or "-1" on error.
int llopen(int fd, LinkLayer connectionParameters);



int readCtrlByte(int fd, unsigned char *CtrlB);
// Send data in buf with size bufSize.
// Return number of chars written, or "-1" on error.
int llwrite(int fd, const unsigned char *buf, int bufSize);



void SMInformationFrame(enum state* currentState, unsigned char byte, unsigned char* controlByte);
int readTransmitterFrame(int fd, unsigned char* buffer);
int verifyFrame(unsigned char* frame, int length);

// Receive data in packet.
// Return number of chars read, or "-1" on error.
int llread(int fd, unsigned char *packet);

// Close previously opened connection.
// if showStatistics == TRUE, link layer should print statistics in the console on close.
// Return "1" on success or "-1" on error.
int llclose(int fd, LinkLayer ll,int showStatistics);

int openSerialPort(const char *port, int baudRate);


#endif // _LINK_LAYER_H_

// Link layer protocol implementation

#include "link_layer.h"
#include "utils.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source

////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////
int llopen(LinkLayer connectionParameters)
{
   
    return 1;
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(const unsigned char *buf, int bufSize)
{
    // TODO

    return 0;
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet)
{
    // TODO

    return 0;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose(int showStatistics)
{
    // TODO

    return 1;
}


int openSerialPort(const char* serialPort) {
    return open(serialPort, O_RDWR | O_NOCTTY);
}

LinkLayer createLinkLayer(const char* serialPort, LinkLayerRole role, int baudRate, int nRetransmissions, int timeout) {
//    LinkLayer ll = malloc(sizeof(LinkLayer));
    LinkLayer ll;
    strcpy(ll.serialPort,serialPort);
    ll.role = role;
    ll.baudRate = baudRate;
    ll.nRetransmissions = nRetransmissions;
    ll.timeout = timeout;

    return ll;
}
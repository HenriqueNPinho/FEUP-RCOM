// Application layer protocol implementation

#include "application_layer.h"
#include "link_layer.h"

#include <stdlib.h>
#include <stdio.h>

void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{
    int fd = openSerialPort(serialPort);
    if (fd<0) {
        printf("Error opening Serial Port");
        exit(EXIT_FAILURE);
    }

    LinkLayerRole llrole;

    if (strcmp(role, "tx")==0) {
        llrole = LlTx;
    } else if (strcmp(role, "rx")==0) {
        llrole = LlRx;
    } else {
        printf("ERROR: Unknown role");
        exit(EXIT_FAILURE);
    }

    LinkLayer ll = createLinkLayer(serialPort, llrole, baudRate, nTries, timeout);

    switch (ll.role)
    {
    case LlTx:
        printf("testar LLTX");
        break;
    case LlRx:
        printf("testar LLRX");
        break;
    default:
        break;
    }

}

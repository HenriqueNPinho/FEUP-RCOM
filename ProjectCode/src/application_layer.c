// Application layer protocol implementation

#include "../include/application_layer.h"



void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{
    int fd = openSerialPort(serialPort, baudRate);

    LinkLayerRole llrole;

    if (strcmp(role, "tx")==0) {
        llrole = LlTx;
    } else if (strcmp(role, "rx")==0) {
        llrole = LlRx;
    } else {
        printf("ERROR: Unknown role\n");
        exit(EXIT_FAILURE);
    }

    LinkLayer ll = createLinkLayer(serialPort, llrole, baudRate, nTries, timeout);

    llopen(fd, ll);

    switch (ll.role)
    {
    case LlTx:
        printf("testar LLTX\n");
        //sendfile();
        break;
    case LlRx:
        printf("testar LLRX\n");
        //receivefile();
        break;
    default:
        break;
    }

}

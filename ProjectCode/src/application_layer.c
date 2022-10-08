// Application layer protocol implementation

#include "../include/application_layer.h"




void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{
    int fd = openSerialPort(serialPort);
    if (fd<0) {
        printf("Error opening Serial Port");
        exit(EXIT_FAILURE);
    }
    else{
        printf("FD PORT: %d\n", fd);
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

    //llopen(ll);

    switch (ll.role)
    {
    case LlTx:
        printf("testar LLTX");
        //sendfile();
        break;
    case LlRx:
        printf("testar LLRX");
        //receivefile();
        break;
    default:
        break;
    }

}

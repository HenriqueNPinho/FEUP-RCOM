// Application layer protocol implementation

#include "../include/application_layer.h"



void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{
    int fd = openSerialPort(serialPort, baudRate);

    LinkLayerRole llrole;

    if (strcmp(role, "tx")==0) {
        llrole = LlTx;
        printf("Transmitter mode engaged\n");
    } else if (strcmp(role, "rx")==0) {
        llrole = LlRx;
        printf("Receiver mode engaged\n");
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
        if (receivefile(fd,filename) != 0) {
            printf("ERROR RECEIVING FILE... :(\n");
            exit(EXIT_FAILURE);
        }
        break;
    default:
        break;
    }

}


int readControlPacket(unsigned char b, unsigned char* packet){
    int packetIndex = 1;
    int fileNameSize = 0;
    int fileSize = 0;
    char* fileName;


    if (b == CONTROL_BYTE_START) {

        if (packet[packetIndex] == FILE_NAME_BYTE) {

            packetIndex++;
            fileNameSize = packet[packetIndex];
            fileName = (char*) malloc(packet[packetIndex]+1);
            packetIndex++;

            for (int i = 0; i < fileNameSize; i++) {
                fileName[i] = packet[packetIndex++];

                if (i == fileNameSize-1) {
                    fileName[fileNameSize] = '\0';
                }
            }
            
        }

        /*if(packet[packetIndex] == FILE_SIZE_FLAG){ //Ainda estou a tentar entender o que se passa aqui para calcular o tamanho

            packetIndex+=2;
            fileSize += packet[packetIndex] << 24;
            packetIndex++;
            fileSize += packet[packetIndex] << 16;
            packetIndex++;
            fileSize += packet[packetIndex] << 8;
            packetIndex++;
            fileSize += packet[packetIndex];
        
        }*/

        //TO DO 
        //Guardar o nome do ficheiro e o tamanho para comparar no fim se bate certo... mas onde para conseguir aceder mais tarde??
        //Como sou o receiver tenho de abrir um sitio novo para meter o ficheio, mas onde meto essa info para os data packets?

    } else if (b== CONTROL_BYTE_END) {

        if (packet[packetIndex] == FILE_NAME_BYTE) {

            packetIndex++;
            fileNameSize = packet[packetIndex];
            fileName = (char*) malloc(packet[packetIndex]+1);
            packetIndex++;

            for (int i = 0; i < fileNameSize; i++) {
                fileName[i] = packet[packetIndex++];

                if (i == fileNameSize-1) {
                    fileName[fileNameSize] = '\0';
                }
            }

        }

        /*if(packet[packetIndex] == FILE_SIZE_FLAG){ //Ainda estou a tentar entender o que se passa aqui para calcular o tamanho
            packetIndex+=2;
            fileSize += packet[packetIndex] << 24;
            packetIndex++;
            fileSize += packet[packetIndex] << 16;
            packetIndex++;
            fileSize += packet[packetIndex] << 8;
            packetIndex++;
            fileSize += packet[packetIndex];
            
        }*/

        //TO DO
        //Comparar se o tamnaho e nome do fichiero sao iguais quando se iniciou

    }
    
    return 0;
}


int receivefile(int fd, const char* filename) {
    unsigned char buffer[256]; //como é que sabemos qual é o tamnaho do pacote para mandar? decidimos nos? 
    int done = 0;

    while (!done) {
        //llread() para o buffer
        //no llread como é que lemos a partir da porta? nao da para a passar
        if (buffer[0] == CONTROL_BYTE_START) {
            //readControlPacket(CONTROL_BYTE_START, buffer);
        }

        if (buffer[0] == CONTROL_BYTE_DATA) {

        }

        if (buffer[0] == CONTROL_BYTE_END) {
            //readControlPacket(CONTROL_BYTE_END, buffer);
            done = 1;
        }
    }

    return 0;
}
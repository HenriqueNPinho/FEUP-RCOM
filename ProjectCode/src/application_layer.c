// Application layer protocol implementation

#include "../include/application_layer.h"


LinkLayerRole llrole;
LinkLayer ll; //l27
ControlPacketInformation packetInfo;


void createLinkLayer(const char* serialPort, LinkLayerRole role, int baudRate, int nRetransmissions, int timeout) {
//    LinkLayer ll = malloc(sizeof(LinkLayer));
   
    strcpy(ll.serialPort,serialPort);
    ll.role = role;
    ll.baudRate = baudRate;
    ll.nRetransmissions = nRetransmissions;
    ll.timeout = timeout;

}

int readFileInformation(const char* fileName){

    int fd;
    struct stat status;

    if((fd = open(fileName,O_RDONLY)) < 0){
        perror("Error opening file!\n");
        return -1;
    }

    if(stat(fileName,&status) < 0){
        perror("Error reading file information!\n");
        return -1;
    }

    packetInfo.fileName = fileName;
    packetInfo.fileSize = status.st_size;

    return 0;
}


int sendControlPacket(unsigned char controlByte){
    // flag start - flag nome - tamanho nome- nome - flag size- tamanho size- size - 
    unsigned char packet[5 + sizeof(packetInfo.fileName)+  sizeof(packetInfo.fileSize)];
    int pIndex=0;

    packet[pIndex]=controlByte;
    pIndex++;


    packet[pIndex]=FILE_NAME_BYTE;
    pIndex++;

    
    packet[pIndex]= sizeof(packetInfo.fileName);
    pIndex++;

    for(int i = 0; i< sizeof(packetInfo.fileName);i++){
        packet[pIndex]= packetInfo.fileName[i];
        pIndex++;
    }
}
int sendFile(const char* filename){
    if(readFileInformation(filename)<0){
        printf("Could not read file information\n");
        return -1;
    }
    /*if(sendControlPacket(flag start)<0)  //start flag
     if()   //send file
    if()     //end flag
    */
}


void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{
    int fd = openSerialPort(serialPort, baudRate);

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

    createLinkLayer(serialPort, llrole, baudRate, nTries, timeout);

    llopen(fd, ll);

    switch (ll.role)
    {
    case LlTx:
        printf("testar LLTX\n");
        //sendFile(filename);
        break;
    case LlRx:
        //printf("sou o receiver");
        if (receiveFile(fd,filename) != 0) {
            printf("ERROR RECEIVING FILE... :(\n");
            exit(EXIT_FAILURE);
        }
        break;
    default:
        break;
    }

    llclose(fd,ll);

}


int readControlPacket(unsigned char controlByte, unsigned char* packet, const char* filename){
    int packetIndex = 1;
    int fileNameSize = 0;
    int fileSize = 0;
    char* fileNameReceived;


    if (controlByte == CONTROL_BYTE_START) {

        if (packet[packetIndex] == FILE_NAME_BYTE) {

            packetIndex++;
            fileNameSize = packet[packetIndex];
            fileNameReceived = (char*) malloc(packet[packetIndex]+1);
            packetIndex++;

            for (int i = 0; i < fileNameSize; i++) {
                fileNameReceived[i] = packet[packetIndex++];

                if (i == fileNameSize-1) {
                    fileNameReceived[fileNameSize] = '\0';
                }
            }
            
        }

        if(packet[packetIndex] == FILE_SIZE_BYTE){ //Ainda estou a tentar entender o que se passa aqui para calcular o tamanho -> otimizaçao 400>>8 = 15

            packetIndex+=2;
            fileSize += packet[packetIndex] << 24;
            packetIndex++;
            fileSize += packet[packetIndex] << 16;
            packetIndex++;
            fileSize += packet[packetIndex] << 8; 
            packetIndex++;
            fileSize += packet[packetIndex];
        
        }


        if (strcmp(filename,fileNameReceived) == 0) {
            packetInfo.fileName = fileNameReceived;
            packetInfo.fileSize = fileSize;
        } else {
            printf("Error file name unknown at start...\n");
            exit(EXIT_FAILURE);
        }
        


    } else if (controlByte== CONTROL_BYTE_END) {

        if (packet[packetIndex] == FILE_NAME_BYTE) {

            packetIndex++;
            fileNameSize = packet[packetIndex];
            fileNameReceived = (char*) malloc(packet[packetIndex]+1);
            packetIndex++;

            for (int i = 0; i < fileNameSize; i++) {
                fileNameReceived[i] = packet[packetIndex++];

                if (i == fileNameSize-1) {
                    fileNameReceived[fileNameSize] = '\0';
                }
            }

        }

        if(packet[packetIndex] == FILE_SIZE_BYTE){ 
            packetIndex+=2;
            fileSize += packet[packetIndex] << 24;
            packetIndex++;
            fileSize += packet[packetIndex] << 16;
            packetIndex++;
            fileSize += packet[packetIndex] << 8;
            packetIndex++;
            fileSize += packet[packetIndex];
            
        }

        if (strcmp(packetInfo.fileName,fileNameReceived) != 0) {
            printf("Different file name at end...\n");
        } 
        if (packetInfo.fileSize != fileSize) {
            printf("Different file size at end...\n");
        }

    }

    
    return 0;
}


int receiveFile(int fd, const char* filename) {
    unsigned char buffer[256]; //como é que sabemos qual é o tamnaho do pacote para mandar? decidimos nos? 
    int done = 0;

    while (done==0) {
        printf("estou fodido\n");
        //llread() para o buffer
        //no llread como é que lemos a partir da porta? nao da para a passar
        if (buffer[0] == CONTROL_BYTE_START) {
            readControlPacket(CONTROL_BYTE_START, buffer, filename);
        }

        if (buffer[0] == CONTROL_BYTE_DATA) {

        }

        if (buffer[0] == CONTROL_BYTE_END) {
            //readControlPacket(CONTROL_BYTE_END, buffer, filename);
            done = 1;
        }
    }

    return 0;
}
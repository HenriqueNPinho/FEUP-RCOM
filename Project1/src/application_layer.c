// Application layer protocol implementation

#include "../include/application_layer.h"


LinkLayerRole llrole;
LinkLayer ll; //l27
ControlPacketInformation packetInfo;


void createLinkLayer(int fd,const char* serialPort, LinkLayerRole role, int baudRate, int nRetransmissions, int timeout, int packetSize) {
   
    strcpy(ll.serialPort,serialPort);
    ll.fdPort=fd;
    ll.role = role;
    ll.baudRate = baudRate;
    ll.nRetransmissions = nRetransmissions;
    ll.timeout = timeout;
    ll.packetSize=packetSize;

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

    packetInfo.fdFile=fd;
    packetInfo.fileName = fileName;
    packetInfo.fileSize = status.st_size;

    return 0;
}


int sendControlPacket(unsigned char controlByte){
    // flag start - flag nome - tamanho nome- nome - flag size- tamanho size- size - 
    unsigned char packet[5 + strlen(packetInfo.fileName)+  sizeof(packetInfo.fileSize)];
    int pIndex=0;

    packet[pIndex++]=controlByte;

    packet[pIndex++]=FILE_NAME_BYTE;

    //printf("\nSize of: %ld \nFilename: %s\n\n", strlen(packetInfo.fileName), packetInfo.fileName);
    packet[pIndex++]= strlen(packetInfo.fileName);
   

    for(size_t i = 0; i< strlen(packetInfo.fileName);i++){
        packet[pIndex++]= packetInfo.fileName[i];
     
    }

    packet[pIndex++]=FILE_SIZE_BYTE;

    packet[pIndex++]=sizeof(packetInfo.fileSize);


    packet[pIndex++]= (packetInfo.fileSize >>24 ) & BYTE_MASK;

    packet[pIndex++]= (packetInfo.fileSize >>16 ) & BYTE_MASK;
 
    packet[pIndex++]= (packetInfo.fileSize >>8 ) & BYTE_MASK;
  
    packet[pIndex++]= (packetInfo.fileSize) & BYTE_MASK;
  

    if(llwrite(ll.fdPort,packet, pIndex)<pIndex){
        printf("error writing control packet\n");
        return -1;
    }
    return 0;
}
int sendDataPacket(){

    int numPacketsSent = 0;
    int numPacketsToSend = packetInfo.fileSize/ll.packetSize;       // numero m??ximo de de octetos num packet
    unsigned char buffer[ll.packetSize];
    int bytesRead = 0;
    int length = 0;
    
    if(packetInfo.fileSize%ll.packetSize != 0){
        numPacketsToSend++;
    }
    while(numPacketsSent < numPacketsToSend){

        if((bytesRead = read(packetInfo.fdFile,buffer,ll.packetSize)) < 0){
            printf("Error reading file\n");
        }
        unsigned char packet[4+ll.packetSize];
        packet[0] = CONTROL_BYTE_DATA;
        packet[1] = numPacketsSent % 255;
        packet[2] = bytesRead / 256;
        packet[3] = bytesRead % 256;
        memcpy(&packet[4],buffer,bytesRead);
        length = bytesRead + 4;

        if(llwrite(ll.fdPort,packet,length) < length){
            printf("Error writing data packet to serial port!\n");
            return -1;
        }
        numPacketsSent++;
    }

    return 0;
}
int sendFile(const char* filename){
    if(readFileInformation(filename)<0){
        printf("Could not read file information\n");
        return -1;
    }
    if(sendControlPacket(CONTROL_BYTE_START)<0){
        printf("Could not send start control packet\n");
        return -1;
    }  //start flag
    if(sendDataPacket()<0){
          printf("Could not send data packet\n");
        return -1;
    }//send file
    if(sendControlPacket(CONTROL_BYTE_END)<0){
        printf("Could not send end control packet\n");
        return -1;
    }    //end flag
    return 0;
}


void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename, int packetSize)
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

    createLinkLayer(fd, serialPort, llrole, baudRate, nTries, timeout, packetSize);

    if (llopen(ll.fdPort, ll)<0) {
        exit(-1);
    }
    
    switch (ll.role) {
        case LlTx:
            //printf("sou o transmitter\n");
            if (sendFile(filename) < 0) {
                printf("\nERROR SENDING FILE\n");
            }
            break;
        case LlRx:

        receiveFile();
      
            break;
        default:
            break;
    }

    
    llclose(ll.fdPort,ll,0);   //////////so para passar na compila??ao
   


    

}


int readControlPacket(unsigned char controlByte, unsigned char* packet){
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

        if(packet[packetIndex] == FILE_SIZE_BYTE){ //Ainda estou a tentar entender o que se passa aqui para calcular o tamanho -> otimiza??ao 400>>8 = 15

            packetIndex+=2;
            fileSize += packet[packetIndex++] << 24;
            fileSize += packet[packetIndex++] << 16;
            fileSize += packet[packetIndex++] << 8;
            fileSize += packet[packetIndex];
        
        }

        packetInfo.fileName = fileNameReceived;
        packetInfo.fileSize = fileSize;

        packetInfo.fdFile = open(fileNameReceived,O_WRONLY | O_CREAT | O_APPEND, 0664);


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
            fileSize += packet[packetIndex++] << 24;
            fileSize += packet[packetIndex++] << 16;
            fileSize += packet[packetIndex++] << 8;
            fileSize += packet[packetIndex];
            
        }

        if (strcmp(packetInfo.fileName,fileNameReceived) != 0) {
            printf("\n\nStart packet and end packet have different file names \n");
            printf("\n %s %s \n", packetInfo.fileName, fileNameReceived);
        } 
        if (packetInfo.fileSize != fileSize) {
            printf("\n\nStart packet and end packet have different file size\n");
        }
    
    }

    
    return 0;
}

int processDataPacket(unsigned char* buffer) {

    int infoSize = 256*buffer[2]+buffer[3];
 
    write(packetInfo.fdFile,buffer+4,infoSize);
    return 0;
}


int receiveFile() {
    unsigned char buffer[ll.packetSize+4]; //como ?? que sabemos qual ?? o tamnaho do pacote para mandar? decidimos nos? 
    int done = 0;
    int lastSequenceNumber = -1;
    int currentSequenceNumber;
 
    while (done==0) {
      /*  if (llread(ll.fdPort, buffer)==0) {
            continue;
        }*/ //para o buffer
  
        llread(ll.fdPort, buffer);
        
        if (buffer[0] == CONTROL_BYTE_START) {
            readControlPacket(CONTROL_BYTE_START, buffer);
        }

        if (buffer[0] == CONTROL_BYTE_DATA) {
           
            currentSequenceNumber = (int)(buffer[1]);
            if(lastSequenceNumber >= currentSequenceNumber && lastSequenceNumber != 254) {continue;}
            
            processDataPacket(buffer);
            lastSequenceNumber = currentSequenceNumber;
        }

        if (buffer[0] == CONTROL_BYTE_END) {
            readControlPacket(CONTROL_BYTE_END, buffer);
            done = 1;
        }
    }
    close(packetInfo.fdFile);
    return 0;
}

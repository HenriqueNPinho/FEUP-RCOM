// Link layer protocol implementation

#include "../include/link_layer.h"

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source
struct termios oldtio;

int alarmFlag; 
int alarmCount = 0;

// Alarm function handler
void alarmHandler(int signal)
{
    alarmFlag = 1;
    alarmCount++;
    printf("-Time out: %d.\n", alarmCount);
}
void startAlarm(){
    
    struct sigaction sa;
	sa.sa_handler = &alarmHandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
    alarmFlag=0;
	sigaction(SIGALRM, &sa, NULL);

    alarm(3); 
 

}
void disableAlarm(){
    struct sigaction sa;
    sa.sa_handler = NULL;

    sigaction(SIGALRM, &sa, NULL);
    alarmFlag=0;
  alarm(0);
}


////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////


int ControlByteCheck(unsigned char b){
    return b==CONTROL_BYTE_SET || b==CONTROL_BYTE_UA || b==CONTROL_BYTE_DISC 
    || b== CONTROL_BYTE_RR0 || b == CONTROL_BYTE_RR1 || b == CONTROL_BYTE_REJ0 || b == CONTROL_BYTE_REJ1;
}
void SMresponse(enum state *currState, unsigned char b, unsigned char* controlb){
    switch (*currState)
    {
    case START:
  
        if(b==FLAG)
            *currState=FLG_RCV;
        break;

    case FLG_RCV:
        if(b==ADDRESS_FIELD)
            *currState=A_RCV;
        else if(b!=FLAG)
            *currState=START;
        break;

    case A_RCV:
        if(ControlByteCheck(b)){
            *currState=C_RCV;
            *controlb=b; //para fazer ou exclusivo
        }
        else if(b==FLAG){
            *currState=FLG_RCV;
        }
        else{
            *currState=START;
        }
        break;
    
    case C_RCV:
        if(b== (ADDRESS_FIELD ^(*controlb))){
            *currState=BCC_OK;
        }
         else if(b==FLAG){
            *currState=FLG_RCV;
        }
        else{
            *currState=START;
        }
        break;
    case BCC_OK:
        if(b==FLAG){
            *currState=STOP;
        }
        else{
            *currState=START;
        }
    break;
    case STOP:
        break;
    default:
        break;
    }
}

void readReceiverResponse(int fd){
    unsigned char b, controlb;
    enum state state=START;
    while(state!=STOP && alarmFlag==0){
        if (read(fd,&b, 1)<0) {
            printf("-Error reading receiver response.\n\n");
        }
        SMresponse(&state, b, &controlb);
    }

}

void readTransmitterResponse(int fd){

    unsigned char b, controlb;
    enum state state=START;
       
    while(state!=STOP){
        if (read(fd,&b, 1)<0) {
            printf("-Error reading transmitter response.\n\n");
        }
        SMresponse(&state, b, &controlb);
    }

}

int llopen(int fd, LinkLayer connectionParameters)
{
    printf("\n-----ESTABLISHING CONNECTION-----\n\n");
    if (connectionParameters.role==TRANSMITTER) {
        unsigned char  ctrlFrame[5];
        ctrlFrame[0]=FLAG;
        ctrlFrame[1]=ADDRESS_FIELD;
        ctrlFrame[2]=CONTROL_BYTE_SET;
        ctrlFrame[3]=ctrlFrame[1]^ctrlFrame[2];
        ctrlFrame[4]=FLAG;

        do{
            write(fd, ctrlFrame, 5);
            printf("-SET Sent.\n");
            alarmFlag= 0;
            startAlarm();
            readReceiverResponse(fd);
            if(!alarmFlag)
                printf("-UA Received.\n");
        } while (alarmCount<MAX_TRIES && alarmFlag);
        disableAlarm();
        
        if(alarmCount>=MAX_TRIES){
            printf("-Max Tries Exceeded.\n");
            printf("--Could not establish connection.\n");
            printf("----------------------------------\n");
            return -1;
        }

    } else if (connectionParameters.role==RECEIVER) {
        readTransmitterResponse(fd);
        printf("-SET received.\n");
        
        unsigned char  ctrlFrame[5];
        ctrlFrame[0]=FLAG;
        ctrlFrame[1]=ADDRESS_FIELD;
        ctrlFrame[2]=CONTROL_BYTE_UA;
        ctrlFrame[3]=ctrlFrame[1]^ctrlFrame[2];
        ctrlFrame[4]=FLAG;
        write(fd, ctrlFrame, 5);
        printf("-UA Sent.\n");

    } else {
        printf("-Unknown flag.\n");
        printf("--Could not establish connection.\n");
        printf("----------------------------------\n");
        return -1;
    }
    printf("\n-----CONNECTION ESTABLISHED-----\n\n");
    return 0;
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int ns=0;

int readCtrlByte(int fd, unsigned char *CtrlB){
  unsigned char b;
  enum state state= START;

  while(state!=STOP && alarmFlag==0){    //ler a trama de controlo
    if(read(fd,&b,1)<0){
      perror("Error reading byte of the receiver response");
    }
       SMresponse(&state,b,CtrlB);
  }

  if(*CtrlB==CONTROL_BYTE_RR0 && ns==1){
    printf("Received postive ACK 0\n");
    return 0;
  }
  else if(*CtrlB==CONTROL_BYTE_RR1 && ns==0){
     printf("Received postive ACK 1\n");
      return 0;
  }
  else if(*CtrlB==CONTROL_BYTE_REJ0 && ns==1){
    printf("Received negative ACK 0\n");
    return -1;
  }
  else if(*CtrlB==CONTROL_BYTE_REJ1 && ns==0){
    printf("Received negative ACK 1\n");
    return -1;
  }
  else {
     return -1;
  }
  return 0;

}

int llwrite(int fd, const unsigned char *buf, int bufSize)
{  //FLAG - A- C - BCC1 - Data - BCC2 - FLAG
  //               a^c            
    int unsigned nChars = 0;
    alarmCount = 0;

    do{
        unsigned char frame[2*bufSize+7]; //2* in case we need to stuff every byte

        frame[0]=FLAG;  
        frame[1]=ADDRESS_FIELD;
        
        if(ns==0){
            frame[2]=CONTROL_BYTE_0;
        }
        else if(ns==1){
            frame[2]=CONTROL_BYTE_1;
        }

        frame[3]= frame[1] ^ frame[2];  //bcc1

        int fIndex=4; 

        unsigned char bcc2 = 0x00;
        for(size_t i=0; i< bufSize;i++){
            bcc2^=buf[i];

            if(buf[i]==FLAG || buf[i]==ESC_BYTE){ //slide 13, ponto 1&2
                frame[fIndex++]=ESC_BYTE;
           
                frame[fIndex++]=buf[i]^STUFFING_BYTE;
           
            }
            else{
                frame[fIndex++]=buf[i];
              
            }
        }

        if(bcc2==FLAG || bcc2 == ESC_BYTE){  //slide 13, ponto 1&2
            frame[fIndex++]=ESC_BYTE;
            frame[fIndex++]=bcc2^STUFFING_BYTE;
          
        }
        else{
            frame[fIndex++]=bcc2;
           
        }
        frame[fIndex]=FLAG;

        nChars = write(fd,frame,fIndex+1);
        printf("Sent frame with sequence number %d\n\n",ns);
        printf("wrote %d bytes\n",nChars);

        startAlarm();
        unsigned char CtrlByte;
        if(readCtrlByte(fd, &CtrlByte)==-1){
        disableAlarm();
        alarmFlag=1;  //tentar enviar outra vez se der erro
        }
        
    } while (alarmCount < MAX_TRIES && alarmFlag);
    disableAlarm();

    if(ns==0){
        ns=1;
    }
    else if(ns==1){
        ns=0;
    }

    if(alarmCount>=MAX_TRIES){
        printf("Max tries exceeded\n");
        return -1;
    }

   return nChars;
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////

void SMInformationFrame(enum state* currentState, unsigned char byte, unsigned char* controlByte) {
    switch(*currentState){
        case START:
            if(byte == FLAG)    
                *currentState = FLG_RCV;
            break;
        case FLG_RCV:
            if(byte == ADDRESS_FIELD)   
                *currentState = A_RCV;
            else if(byte != FLAG)
                *currentState = START;
            break;
        case A_RCV:
            if(byte == CONTROL_BYTE_0 || byte == CONTROL_BYTE_1){
              *currentState = C_RCV;
              *controlByte = byte;
            }
            else if(FLAG == 0x7E){
                *currentState = FLG_RCV;
            }
            else{
                *currentState = START;
            }
            break;
        case C_RCV:
            if(byte == (ADDRESS_FIELD^(*controlByte)))
              *currentState = BCC_OK;
            else if(byte == FLAG)
              *currentState = FLG_RCV;
            else
              *currentState = START;
            
            break;
        case BCC_OK:
            if(byte != FLAG)
              *currentState = DATA_RCV;
            break;
        case DATA_RCV:
            if(byte == FLAG)
              *currentState = STOP;
            break;
        case STOP:
            break;
    }
}

int readTransmitterFrame(int fd, unsigned char* buffer) {
   
    int pos=0;
    unsigned char byte;
    unsigned char controlByte;
    enum state state = START;
    while (state != STOP) {
        if(read(fd,&byte,1)<0) {
            printf("erro no byte 'readtransmitterframe'\n");
        }
        SMInformationFrame(&state,byte,&controlByte);
        buffer[pos] = byte;
        pos++;
    }
         
    return pos;
}


int verifyFrame(unsigned char* frame, int length) {
    unsigned char addressField = frame[1];
    unsigned char controlByte = frame[2];
    unsigned char bcc1 = frame[3];
    unsigned char bcc2 = frame[length-2];
    unsigned char aux = 0x00;

    //verify if bcc1 is correct
    if (controlByte != CONTROL_BYTE_0 && controlByte != CONTROL_BYTE_1) {
        printf("Error in the protocol! (bcc1)\n");
        return -1;
    } else if(bcc1 == (addressField^controlByte)){
        //verify if bcc2 is correct
        for (size_t i = 4; i < length-2; i++) {
            aux^=frame[i];
        }

        if(bcc2 != aux) {
            printf("Error in the data! (bcc2)\n");
            return -2;
        }
    }

        return 0;

}

int llread(int fd,unsigned char *packet)
{
    
    int received = 0;
    int length = 0;
    unsigned char controlByte;
    unsigned char auxBuffer[131087];
    int packetIndex = 0;
    alarmCount = 0;

    while (received == 0) {
        length = readTransmitterFrame(fd,auxBuffer);
        printf("frame received\n");

        if (length > 0) {
            unsigned char originalFrame[2*length+7];

            for(size_t i=0; i<4; i++){
                originalFrame[i]=auxBuffer[i];
            }
            
            int originalFrameIndex = 4;
            int escapeByteFound = FALSE;

            for (int i = 4; i < length-1; i++) {
                if (auxBuffer[i] == ESC_BYTE) {
                    escapeByteFound = TRUE;
                    continue;
                } else if (auxBuffer[i] == (FLAG^STUFFING_BYTE) && escapeByteFound) {
                    originalFrame[originalFrameIndex++] = FLAG;
                    escapeByteFound = FALSE;
                } else if (auxBuffer[i] == (ESC_BYTE^STUFFING_BYTE) && escapeByteFound) {
                    originalFrame[originalFrameIndex++] = ESC_BYTE;
                    escapeByteFound = FALSE;
                } else {
                    originalFrame[originalFrameIndex++] = auxBuffer[i];
                }
            }
     
            originalFrame[originalFrameIndex] = auxBuffer[length-1];
            controlByte = originalFrame[2];

            if (verifyFrame(originalFrame,originalFrameIndex+1) != 0) {
                if (controlByte == CONTROL_BYTE_0) {
                    printf("Rejected frame with 1 as sequence number\n");
                    unsigned char frame[5];
                    frame[0] = FLAG;
                    frame[1] = ADDRESS_FIELD;
                    frame[2] = CONTROL_BYTE_REJ0;
                    frame[3] = frame[1] ^ frame[2];
                    frame[4] = FLAG;
                    int res = write(fd,frame,5);
                    printf("Sent negative ACK 0 (REJ0) - %d bytes written\n",res);
                } else if (controlByte == CONTROL_BYTE_1) {
                    printf("Rejected frame with 0 as sequence number\n");
                    unsigned char frame[5];
                    frame[0] = FLAG;
                    frame[1] = ADDRESS_FIELD;
                    frame[2] = CONTROL_BYTE_REJ1;
                    frame[3] = frame[1] ^ frame[2];
                    frame[4] = FLAG;
                    int res = write(fd,frame,5);
                    printf("Sent negative ACK 1 (REJ1) - %d bytes written\n",res);
                }

                return 0;

            } else {
                for (size_t i = 4; i < originalFrameIndex-1; i++) {
                     
                    packet[packetIndex++] = originalFrame[i];
                }
               
                if (controlByte == CONTROL_BYTE_0) {
                    unsigned char frame[5];
                    frame[0] = FLAG;
                    frame[1] = ADDRESS_FIELD;
                    frame[2] = CONTROL_BYTE_RR1;
                    frame[3] = frame[1] ^ frame[2];
                    frame[4] = FLAG;
                    int res = write(fd, frame, 5);
                    printf("Sent positive ACK 1 (RR1) - %d bytes written\n",res);
                } else if (controlByte == CONTROL_BYTE_1) {
                    unsigned char frame[5];
                    frame[0] = FLAG;
                    frame[1] = ADDRESS_FIELD;
                    frame[2] = CONTROL_BYTE_RR0;
                    frame[3] = frame[1] ^ frame[2];
                    frame[4] = FLAG;
                    int res = write(fd, frame, 5);
                    printf("Sent positive ACK 0 (RR0) - %d bytes written\n",res);
                }
                received = 1;
            }

        }
    }
    

    return packetIndex;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose(int fd, LinkLayer ll, int showStatistics) //Depois meter o "int showStatistics"
{
    printf("\n-----CLOSING CONNECTION-----\n\n");

    if (ll.role == TRANSMITTER) {
        alarmCount = 0;
        unsigned char  ctrlFrame[5];
        ctrlFrame[0]=FLAG;
        ctrlFrame[1]=ADDRESS_FIELD;
        ctrlFrame[2]=CONTROL_BYTE_DISC;
        ctrlFrame[3]=ctrlFrame[1]^ctrlFrame[2];
        ctrlFrame[4]=FLAG;

        do{
            int res = write(fd, ctrlFrame, 5);
            printf("-DISC Sent - %d bytes written.\n",res);
            alarmFlag = 0;
            startAlarm();        
            readReceiverResponse(fd);
            printf("estou a tentar receber a resposta do R\n");
        } while (alarmCount<MAX_TRIES && alarmFlag);

        if (alarmFlag==0) {
            printf("-DISC received.\n");
        }

        disableAlarm();

        if (alarmCount>=MAX_TRIES) {
            printf("-Max tries exceeded.\n");
            printf("--Could not close connection.\n");
            printf("------------------------------\n");
            return -1;
        } else {
            unsigned char  ctrlFrame[5];
            ctrlFrame[0]=FLAG;
            ctrlFrame[1]=ADDRESS_FIELD;
            ctrlFrame[2]=CONTROL_BYTE_UA;
            ctrlFrame[3]=ctrlFrame[1]^ctrlFrame[2];
            ctrlFrame[4]=FLAG;

            int res = write(fd, ctrlFrame, 5);
            printf("-Last UA Sent - %d bytes written\n", res);
            sleep(1);
        }

    } else if (ll.role == RECEIVER) {
        
        readTransmitterResponse(fd);
        printf("-DISC received.\n");


        unsigned char  ctrlFrame[5];
        ctrlFrame[0]=FLAG;
        ctrlFrame[1]=ADDRESS_FIELD;
        ctrlFrame[2]=CONTROL_BYTE_DISC;
        ctrlFrame[3]=ctrlFrame[1]^ctrlFrame[2];
        ctrlFrame[4]=FLAG;
        int res = write(fd, ctrlFrame, 5);
        printf("-DISC Sent - %d bytes written.\n", res);
    }

    
    if (showStatistics == TRUE) {
        printf("Statistics\n");
    }

    tcflush(fd, TCIOFLUSH);

	if (tcsetattr(fd, TCSANOW, &oldtio) == -1) {
		perror("tcsetattr");
		exit(-1);
	}

    printf("-Estou no fim.\n");
    close(fd);

   
    printf("\n-----CONNECTION CLOSED-----\n\n");
    return 0;
}





int openSerialPort(const char* port, int baudRate)
{
  struct termios newtio;
  int fd;

   fd=open(port,O_RDWR | O_NOCTTY);
    if (fd <0) {perror(port); exit(-1);}

    if (tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
        perror("tcgetattr");
        exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = baudRate | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 10;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */

    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }
     printf("New termios structure set with baudRate: in:%d | out:%d\n", newtio.c_ispeed, newtio.c_ospeed);

    return fd;
}
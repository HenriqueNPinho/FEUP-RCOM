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
    printf("Alarm #%d\n", alarmCount);
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
    return b==CONTROL_BYTE_SET | b==CONTROL_BYTE_UA | b==CONTROL_BYTE_DISC;
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
        if(read(fd,&b, 1)<0){
            printf("error reading receiver response\n");
        }    
        SMresponse(&state, b, &controlb);
    }

}

void readTransmiterResponse(int fd){

    unsigned char b, controlb;
    enum state state=START;
       
    while(state!=STOP){
      
        if(read(fd,&b, 1)<0){
            perror("error reading transmiter response\n");
        }
         
        SMresponse(&state, b, &controlb);
    }

}

int llopen(int fd, LinkLayer connectionParameters)
{
       if(connectionParameters.role==TRANSMITTER){
        unsigned char  ctrlFrame[5];
        ctrlFrame[0]=FLAG;
        ctrlFrame[1]=ADDRESS_FIELD;
        ctrlFrame[2]=CONTROL_BYTE_SET;
        ctrlFrame[3]=ctrlFrame[1]^ctrlFrame[2];
        ctrlFrame[4]=FLAG;

        do{
            write(fd, ctrlFrame, 5);
            printf("SET Sent\n");
            alarmFlag= 0;
            startAlarm();        
            readReceiverResponse(fd);
            if(!alarmFlag)
                printf("UA received\n");
        }
        while (alarmCount<MAX_TRIES && alarmFlag);
        disableAlarm();
        
        if(alarmCount>MAX_TRIES){
            printf("max tries exceeded\n");
            exit(-1);
        }
    }
    else if(connectionParameters.role==RECEIVER){
        readTransmiterResponse(fd);
        printf("SET received\n");
        
        unsigned char  ctrlFrame[5];
        ctrlFrame[0]=FLAG;
        ctrlFrame[1]=ADDRESS_FIELD;
        ctrlFrame[2]=CONTROL_BYTE_UA;
        ctrlFrame[3]=ctrlFrame[1]^ctrlFrame[2];
        ctrlFrame[4]=FLAG;
        write(fd, ctrlFrame, 5);
        printf("UA Sent\n");

    }
    else{
    printf("Unknown flag\n");
    exit(1);
    } 
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
int llclose(int fd, LinkLayer ll, int showStatistics) //Depois meter o "int showStatistics"
{
    if (ll.role == TRANSMITTER) {
        unsigned char  ctrlFrame[5];
        ctrlFrame[0]=FLAG;
        ctrlFrame[1]=ADDRESS_FIELD;
        ctrlFrame[2]=CONTROL_BYTE_DISC;
        ctrlFrame[3]=ctrlFrame[1]^ctrlFrame[2];
        ctrlFrame[4]=FLAG;

        do{
            write(fd, ctrlFrame, 5);
            printf("DISC Sent\n");
            alarmFlag = 0;
            startAlarm();        
            readReceiverResponse(fd);
            if(!alarmFlag)
                printf("DISC received\n");
        }
        while (alarmCount<MAX_TRIES && alarmFlag);
        disableAlarm();

        if(alarmCount>MAX_TRIES){
            printf("max tries exceeded\n");
            exit(-1);
        } else {
            unsigned char  ctrlFrame[5];
            ctrlFrame[0]=FLAG;
            ctrlFrame[1]=ADDRESS_FIELD;
            ctrlFrame[2]=CONTROL_BYTE_UA;
            ctrlFrame[3]=ctrlFrame[1]^ctrlFrame[2];
            ctrlFrame[4]=FLAG;

            write(fd, ctrlFrame, 5);
            printf("Last UA Sent");
        }

    } else if (ll.role == RECEIVER) {
        readTransmiterResponse(fd);
        printf("DISC received\n");


        unsigned char  ctrlFrame[5];
        ctrlFrame[0]=FLAG;
        ctrlFrame[1]=ADDRESS_FIELD;
        ctrlFrame[2]=CONTROL_BYTE_DISC;
        ctrlFrame[3]=ctrlFrame[1]^ctrlFrame[2];
        ctrlFrame[4]=FLAG;
        write(fd, ctrlFrame, 5);
        printf("DISC Sent\n");

    }

    close(fd);

    if (showStatistics == TRUE) {
        printf("Statistics\n");
    }


    return 1;
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

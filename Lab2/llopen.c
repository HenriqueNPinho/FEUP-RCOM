#include "utils.h"

int alarmEnabled = FALSE;
int alarmCount = 0;


// Alarm function handler
void alarmHandler(int signal)
{
    alarmEnabled = FALSE;
    alarmCount++;

    printf("Alarm #%d\n", alarmCount);
}
void startAlarm(){
    (void)signal(SIGALRM, alarmHandler);

    while (alarmCount < 4)
    {
        if (alarmEnabled == FALSE)
        {
            alarm(3); // Set alarm to be triggered in 3s
            alarmEnabled = TRUE;
        }
    }

}
void disableAlarm(){
    struct sigaction sa;
	sa.sa_handler = NULL;
    sigaction(SIGALRM, &sa, NULL);

  alarm(0);
}

int ControlByteCheck(unsigned char b){
    return b==CONTROL_BYTE_SET | b==CONTROL_BYTE_UA;
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
    print("cona\n");
    unsigned char b, controlb;
    enum state state=START;
    while(state!=STOP){
        if(read(fd,&b, 1)<0){
            perror("error reading receiver response\n");
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

void llopen(int fd, int flag){
    if(flag==TRANSMITTER){
        unsigned char  ctrlFrame[5];
        ctrlFrame[0]=FLAG;
        ctrlFrame[1]=ADDRESS_FIELD;
        ctrlFrame[2]=CONTROL_BYTE_SET;
        ctrlFrame[3]=ctrlFrame[1]^ctrlFrame[2];
        ctrlFrame[4]=FLAG;

        do{
            write(fd, ctrlFrame, 5);
            printf("SET Sent\n");
            startAlarm();
            readReceiverResponse(fd);
            printf("UA received\n");
        }
        while (alarmCount<MAX_TRIES);
        disableAlarm();
        
        if(alarmCount>MAX_TRIES){
            printf("max tries exceeded\n");
            exit(-1);
        }
    }
    else if(flag==RECEIVER){
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
}
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
            printf("Send SET\n");
            alarmCount=0;
            (void)signal(SIGALRM, alarmHandler);
            
        }
        while (/* condition */)
        {
            /* code */
        }
        
    }
    else if(flag==RECEIVER){

    }
    else{
    printf("Unknown flag\n");
    exit(1);
    } 
}
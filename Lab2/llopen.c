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

void readReceiverResponse(int fd){
    unsigned char b, controlb;
    //primeiro estado
    while(//estado  )

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
            startAlarm();
            //read
            
        }
        while (/* <maxtries */);
        disableAlarm();
        
        //if maxtries
            //print max tries exceeded
        
    }
    else if(flag==RECEIVER){

    }
    else{
    printf("Unknown flag\n");
    exit(1);
    } 
}
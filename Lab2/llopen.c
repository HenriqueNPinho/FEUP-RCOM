#include "utils.h"

void llopen(int fd, int flag){
    if(flag==TRANSMITTER){
        unsigned char  ctrlFrame[5];
        ctrlFrame[0]=FLAG;
        ctrlFrame[1]=ADDRESS_FIELD;
        ctrlFrame[2]=CONTROL_BYTE_SET;
        ctrlFrame[3]=ctrlFrame[1]^ctrlFrame[2];
        ctrlFrame[4]=FLAG;

    }
    else if(flag==RECEIVER){

    }
    else{
    printf("Unknown flag\n");
    exit(1);
    } 
}
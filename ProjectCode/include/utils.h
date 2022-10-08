#include <unistd.h>
#include <signal.h>
#include <stdio.h>

#define FALSE 0
#define TRUE 1

#define TRANSMITTER 0 
#define RECEIVER 1

#define FLAG 0x7E
#define ADDRESS_FIELD 0x03
#define CONTROL_BYTE_SET 0x03
#define CONTROL_BYTE_UA 0X07

#define MAX_TRIES 3

enum state{START, FLG_RCV, A_RCV, C_RCV, BCC_OK,STOP};


#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <termios.h>


#define FALSE 0
#define TRUE 1

#define TRANSMITTER 0 
#define RECEIVER 1

#define FLAG 0x7E
#define ADDRESS_FIELD 0x03
#define CONTROL_BYTE_SET 0x03
#define CONTROL_BYTE_UA 0X07

#define CONTROL_BYTE_START 0x02
#define CONTROL_BYTE_END 0x03
#define CONTROL_BYTE_DATA 0x01

#define FILE_NAME_BYTE 0x01

#define MAX_TRIES 3

enum state{START, FLG_RCV, A_RCV, C_RCV, BCC_OK,STOP};

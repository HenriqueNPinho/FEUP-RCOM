#include "alarm.c"
#include <unistd.h>
#include <signal.h>
#include <stdio.h>

#define FALSE 0
#define TRUE 1

#define TRANSMITTER 1 
#define RECEIVER 0

#define FLAG 0x7E
#define ADDRESS_FIELD 0x03
#define CONTROL_BYTE_SET 0x03
#define CONTROL_BYTE_UA 0X07
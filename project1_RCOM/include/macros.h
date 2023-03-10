#ifndef MACROS_H
#define MACROS_H

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 // POSIX compliant source

#define BUF_SIZE 256
#define SUP_FRAME_SIZE 5
#define N_TRIES 3
#define TIMEOUT 4
#define MAX_PAYLOAD_SIZE 1000

#define FALSE 0
#define TRUE 1 

#define TRANSMITTER 0
#define RECEIVER 1

#define FLAG 0x7E
#define ESC 0x7D

#define A_EMI 0x03 
#define A_REC 0x01 
#define REC 0x01
#define C_SET 0x03
#define C_UA 0x07
#define C_RR1 0xB5
#define C_RR0 0x05
#define C_REJ1 0x81
#define C_REJ0 0x01
#define C_DISC 0x0B
#define C_I0 0x00
#define C_I1 0XB0

#define PORTS0 "/dev/ttyS10"
#define PORTS1 "/dev/ttyS11"

#define ICTRL 0x01
#define ISTART 0x02
#define IEND 0x03

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#endif

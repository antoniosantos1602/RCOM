#ifndef AUX_H
#define AUX_H

#include "state_machine.h"

typedef enum{
    SET,
    UA,
    RR1,
    REJ1,
    RR0,
    REJ0,
    DISC
} FRAME;

int byteStuffing(int size, const unsigned char* data, unsigned char* frame, unsigned char bcc2);
int createIFrame(int size, const unsigned char *data, int iFrameType, unsigned char *frame);
void createSupFrame(FRAME type, unsigned char *frame, int role);
int receiveSupFrame(int fd, unsigned char *frame, int type, int role);
int receiveIFrame(int fd, unsigned char *frame);
void printFrame(unsigned char * frame, int size);



#endif

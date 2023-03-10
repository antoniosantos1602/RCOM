// Link layer header.
// NOTE: This file must not be changed.

#ifndef _LINK_LAYER_H_
#define _LINK_LAYER_H_

#include "macros.h"
#include "auxfunc.h"
#include "state_machine.h"

typedef enum
{
    LlTx,
    LlRx,
} LinkLayerRole;

typedef struct
{
    char serialPort[50];
    int port; 
    LinkLayerRole role;
    int baudRate;
    int nRetransmissions;   
    int timeout;
    int sequence_number;
} LinkLayer;

struct termios oldtio;

// SIZE of maximum acceptable payload.
// Maximum number of bytes that application layer should send to link layer
#define MAX_PAYLOAD_SIZE 1000

// MISC
#define FALSE 0
#define TRUE 1

// Open a connection using the "port" parameters defined in struct linkLayer.
// Return "1" on success or "-1" on error.
int llopen(LinkLayer *connectionParameters);

// Send data in buf with size bufSize.
// Return number of chars written, or "-1" on error.
int llwrite(const unsigned char *buf, int bufSize, LinkLayer *connectionParameters);

// Receive data in packet.
// Return number of chars read, or "-1" on error.
int llread(unsigned char *packet, LinkLayer *connectionParameters);

// Close previously opened connection.
// if showStatistics == TRUE, link layer should print statistics in the console on close.
// Return "1" on success or "-1" on error.
int llclose(LinkLayer connectionParameters);

#endif // _LINK_LAYER_H_

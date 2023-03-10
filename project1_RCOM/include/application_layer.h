// Application layer protocol header.
// NOTE: This file must not be changed.

#ifndef _APPLICATION_LAYER_H_
#define _APPLICATION_LAYER_H_

#include "link_layer.h"

typedef struct {
  int size;          // file size
  char fileName[255];        // file name
  FILE* file;
} FileInfo;

LinkLayer connectionParameters;
FileInfo file;

// Application layer main function.
// Arguments:
//   serialPort: Serial port name (e.g., /dev/ttyS0).
//   role: Application role {"tx", "rx"}.
//   baudrate: Baudrate of the serial port.
//   nTries: Maximum number of frame retries.
//   timeout: Frame timeout.
//   filename: Name of the file to send / receive.

void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename);

int buildDataPacket(unsigned char *packet, int sequenceNumber, unsigned char *data, int length);
int buildControlPacket(unsigned char control, unsigned char *packet);
int parseDataPacket(unsigned char *packet, unsigned char *data, int sequenceNumber);
int parseControlPacket(unsigned char *packet);
void sendFile();
void receiveFile();


#endif // _APPLICATION_LAYER_H_

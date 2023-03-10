// Link layer protocol implementation

#include "link_layer.h"

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source

////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////
int llopen(LinkLayer *connectionParameters)
{
    int fd = open(connectionParameters->serialPort, O_RDWR | O_NOCTTY);

    connectionParameters->port = fd;
    printf("fd: %d\n", fd);

    if (fd < 0){
        perror(connectionParameters->serialPort);
        return -1;
    }

    struct termios newtio;

    // Save current port settings
    if (tcgetattr(connectionParameters->port, &oldtio) == -1){
        perror("tcgetattr");
        return -1;
    }

    // Clear struct for new port settings
    memset(&newtio, 0, sizeof(newtio));

    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    // Set input mode (non-canonical, no echo,...)
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 0.1; // Inter-character timer unused
    newtio.c_cc[VMIN] = 0;  // Blocking read until 5 chars received

    // VTIME e VMIN should be changed in order to protect with a
    // timeout the reception of the following character(s)

    // Now clean the line and activate the settings for the port
    // tcflush() discards data written to the object referred to
    // by fd but not transmitted, or data received but not read,
    // depending on the value of queue_selector:
    //   TCIFLUSH - flushes data received but not read.
    tcflush(fd, TCIOFLUSH);

    // Set new port settings
    if (tcsetattr(fd, TCSANOW, &newtio) == -1){   
        perror("tcsetattr");
        return -1; 
    }

    if(connectionParameters->role == LlTx){
        unsigned char *SET_frame = (unsigned char*)malloc(SUP_FRAME_SIZE * sizeof(unsigned char));
        unsigned char *UA_frame = (unsigned char*)malloc(SUP_FRAME_SIZE * sizeof(unsigned char));

        createSupFrame(SET, SET_frame, connectionParameters->role);
        printFrame(SET_frame, SUP_FRAME_SIZE);
        write(fd, SET_frame, SUP_FRAME_SIZE);

     
        if(receiveSupFrame(fd, UA_frame, UAFRAME, connectionParameters->role) == 0){
            printf("couldnt receive frame\n");
            return 0;
        }
        printFrame(UA_frame, SUP_FRAME_SIZE);

        free(SET_frame);
        free(UA_frame);
    }
    if(connectionParameters->role == LlRx){
        unsigned char *SET_frame = (unsigned char*)malloc(SUP_FRAME_SIZE * sizeof(unsigned char));
        unsigned char *UA_frame = (unsigned char*)malloc(SUP_FRAME_SIZE * sizeof(unsigned char));

        
        if(receiveSupFrame(fd, SET_frame, SETFRAME, connectionParameters->role) == 0){
            printf("couldnt receive frame\n");
            return 0;
        }
        printFrame(SET_frame, SUP_FRAME_SIZE);

        createSupFrame(UA, UA_frame, connectionParameters->role);
        printFrame(UA_frame, SUP_FRAME_SIZE);
        write(fd, UA_frame, SUP_FRAME_SIZE);
        free(SET_frame);
        free(UA_frame);
    }

    return 1;
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(const unsigned char *buf, int bufSize, LinkLayer *connectionParameters)
{   
    unsigned char *I_frame = (unsigned char*)malloc((bufSize*2+6) * sizeof(unsigned char));
    unsigned char *ACK_frame = (unsigned char*)malloc(SUP_FRAME_SIZE * sizeof(unsigned char));
    int length = createIFrame(bufSize, buf, connectionParameters->sequence_number, I_frame);
    printFrame(I_frame, length);
    printf("length da iframe criada = %d\n", length);
    write(connectionParameters->port, I_frame, length);
    printf("waiting for sup\n");
    if(receiveSupFrame(connectionParameters->port, ACK_frame, ACK, connectionParameters->role) == 0)
        return 0;
    printf("after sup\n");
    printFrame(ACK_frame, SUP_FRAME_SIZE);
    if(ACK_frame[2] == C_RR1)
        connectionParameters->sequence_number = 1;
    else if(ACK_frame[2] == C_RR0)
        connectionParameters->sequence_number = 0;
    else{
        printf("oh no\n");
        return 0;
    }

    free(I_frame);
    free(ACK_frame);

    return bufSize;
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet, LinkLayer *connectionParameters)
{
    unsigned char *I_frame = (unsigned char*)malloc((MAX_PAYLOAD_SIZE*2+6) * sizeof(unsigned char));
    printf("before receiving i frame\n");

    int bytes = receiveIFrame(connectionParameters->port, I_frame);
    if(bytes == 0){
        printf("error iframe\n");
        return 0;
    }
    printf("bytes da iframe recebida = %d\n", bytes);

    printFrame(I_frame, bytes);
    unsigned char *frame = (unsigned char *)malloc(SUP_FRAME_SIZE*sizeof(unsigned char));

    if(I_frame[2] == C_I0 && connectionParameters->sequence_number == 1){
        printf("Resend I1\n");
        bytes = 0;
        createSupFrame(RR1, frame, connectionParameters->role);
    }
    else if (I_frame[2] == C_I1 && connectionParameters->sequence_number == 0){
        printf("Resend I0\n");
        bytes = 0;
        createSupFrame(RR0, frame, connectionParameters->role);
    }
    else if (I_frame[2] == C_I1){
        createSupFrame(RR0, frame, connectionParameters->role);
        connectionParameters->sequence_number = 0;
    }
    else if (I_frame[2] == C_I0){
        createSupFrame(RR1, frame, connectionParameters->role);
        connectionParameters->sequence_number = 1;
    }
    else{
        return 0;
    } 
    printf("writing sup!!!\n");

    write(connectionParameters->port, frame, SUP_FRAME_SIZE);

    printf("putting in the packet\n");
    for(int i = 0; i < bytes; i++){
        packet[i] = I_frame[i+4];
    }
    printFrame(frame, SUP_FRAME_SIZE);
    
    free(I_frame);
    free(frame);
    return bytes;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose(LinkLayer connectionParameters)
{
    if(connectionParameters.role == LlTx){
        unsigned char *frame = (unsigned char *)malloc(SUP_FRAME_SIZE * sizeof(unsigned char));
        unsigned char *RCVframe = (unsigned char *)malloc(SUP_FRAME_SIZE * sizeof(unsigned char));
        unsigned char *UAframe = (unsigned char *)malloc(SUP_FRAME_SIZE * sizeof(unsigned char));

        createSupFrame(DISCFRAME, frame, connectionParameters.role);

        printf("DISC1: ");
        printFrame(frame, SUP_FRAME_SIZE);

        write(connectionParameters.port, frame, SUP_FRAME_SIZE);

        if(receiveSupFrame(connectionParameters.port, RCVframe, DISCFRAME, connectionParameters.role) == 0){
            printf("Fail\n");
            return -1;
        }
        createSupFrame(UA, UAframe, connectionParameters.role);

        printf("UA: ");
        printFrame(UAframe, SUP_FRAME_SIZE);

        write(connectionParameters.port, UAframe, SUP_FRAME_SIZE);
        free(frame);
        free(RCVframe);
        free(UAframe);
    }
    else if(connectionParameters.role == LlRx){
        unsigned char *frame = (unsigned char *)malloc(SUP_FRAME_SIZE * sizeof(unsigned char));
        unsigned char *RCVframe = (unsigned char *)malloc(SUP_FRAME_SIZE * sizeof(unsigned char));
        unsigned char *UAframe = (unsigned char *)malloc(SUP_FRAME_SIZE * sizeof(unsigned char));

        if(receiveSupFrame(connectionParameters.port, frame, DISCFRAME, connectionParameters.role) == 0){
            printf("Fail1\n");
            return 0;
        }

        createSupFrame(DISC, RCVframe, connectionParameters.role);

        printf("DISC2: ");
        printFrame(RCVframe, SUP_FRAME_SIZE);

        write(connectionParameters.port, RCVframe, SUP_FRAME_SIZE);

        if(receiveSupFrame(connectionParameters.port, UAframe, UAFRAME, connectionParameters.role) == 0){
            printf("Fail2\n");
            return 0;
        }
        free(frame);
        free(RCVframe);
        free(UAframe);
    }

    printf("%d\n",connectionParameters.port);
   if (tcsetattr(connectionParameters.port, TCSANOW, &oldtio) == -1){
        perror("Couldn't close port\n");
        return -1;
    }

    return close(connectionParameters.port);
}

#include "macros.h"
#include "auxfunc.h"

//byte stuffing
int byteStuffing(int size, const unsigned char* data, unsigned char* frame, unsigned char bcc2){
    int length = 4;
    unsigned char byte;
    for(int i = 0; i < size; i++){
        byte = data[i];
        if(data[i]==ESC){
            frame[length++] = ESC;
            frame[length] = (ESC ^ 0x20); 
        }
        else if(data[i]==FLAG){
            frame[length++] = ESC;
            frame[length] = (FLAG^0x20);
        }
        else
            frame[length] = byte;
        length++;
    }

    if(bcc2==ESC){
            frame[length++] = ESC;
            frame[length] = (ESC ^ 0x20); 
        }
    else if(bcc2==FLAG){
        frame[length++] = ESC;
        frame[length] = (FLAG^0x20);
    }   
    else 
        frame[length] = byte;

    printf("length after stuffing: %d\n", length);
    return length;
}

//byte destuffing

int byteDestuffing(int size, unsigned char* frame){
    printf("size before destuffing: %d\n", size);
    unsigned char buf[size];
    printf("after buf[size]\n");
    printf("frame[0]: %u\n", frame[0]);
    for(int i = 0; i < size; i++){
        printf("%u ", frame[i]);
        buf[i] = frame[i];
    }
    printf("start destuffing\n");
    unsigned char byte;
    int previousESC = FALSE;
    int length = 4; 
    
    for(int i = 4; i < size; i++){
        byte = frame[i];
        if(byte == ESC){
            previousESC = TRUE;
            continue;
        }
        else if(previousESC){
            if(byte == (ESC ^ 0x20))
                frame[length] = ESC;
            else if(byte == (FLAG ^ 0x20))
                frame[length] = FLAG; 
            previousESC = FALSE;
            length++;
        }
        else{
            frame[length] = buf[i];
            length++;
        }
    }
    printf("final length after destuffing = %d\n", length);
    return length;
}



//createFrameInfo
int createIFrame(int size, const unsigned char *data, int iFrameType, unsigned char *frame){
    int length = 0;
    unsigned char bcc2 = 0x00;
    frame[0] = FLAG;
    frame[1] = A_EMI;
    if(iFrameType == 0)
        frame[2] = C_I0;
    else
        frame[2] = C_I1;

    frame[3] = (frame[2] ^ A_EMI);
    for (int i = 0; i < size; i++){
        bcc2 = bcc2^data[i];
    }
    length = byteStuffing(size, data, frame, bcc2);
    frame[length++] = FLAG;

    if(realloc(frame, length*sizeof(unsigned char)) == NULL){
        printf("Realloc failed\n");
        exit(1);
    }
    printf("length of i frame created %d\n", length);
    return length;
}

//createSupFrame    
void createSupFrame(FRAME type, unsigned char *frame, int role){
    frame[0] = FLAG;
    frame[1] = A_EMI;
    switch (type)
    {
    case SET:
        frame[2] = C_SET;
        frame[3] = (A_EMI ^ C_SET);
        break;
    case UA:
        frame[2] = C_UA;
        if(role == 1){
            frame[3] = (A_EMI ^ C_UA);
        }
        else{
            frame[1] = A_REC;
            frame[3] = (A_REC ^ C_UA);
        }
        break;
    case RR0:
        frame[2] = C_RR0;
        frame[3] = (C_RR0 ^ A_EMI);
        break;
    case RR1:
        frame[2] = C_RR1;
        frame[3] = (C_RR1 ^ A_EMI);
        break;
    case REJ0: 
        frame[2] = C_REJ0;
        frame[3] = (C_REJ0 ^ A_EMI);
        break;
    case REJ1:
        frame[2] = C_REJ1;
        frame[3] = (C_REJ1 ^ A_EMI);
    case DISC:
        frame[2] = C_DISC;
        if(role == 1){
            frame[1] = A_REC;
            frame[3] = (A_REC ^ C_DISC);
        }
        else{
            frame[3] = (A_EMI ^ C_DISC);
        }
    }

    frame[4] = FLAG;
}

//receive SUP frame

int receiveSupFrame(int fd, unsigned char *frame, int type, int role){
    unsigned char byteGetter;
    int length = 0;
    int byte = 0;
    STATE state = START; 
    printf ("Being received: ");
    do{
        byte = read(fd, &byteGetter, 1);
        if(byte == 0)
            continue;
        printf("%u ", byteGetter);
        state = machine(state, type, byteGetter, role);
        if(state == START)
            return 0; //needs to resend
        
        frame[length] = byteGetter;
        length++;
    }while(state!=STOP);
    printf("\n");

    printf("FRAME RECEIVED: ");
    printFrame(frame, length);
    return 1;
}

//receive info frame

int receiveIFrame(int fd, unsigned char *frame){
    unsigned char byteGetter;
    int byte = 0, bytes = 0;
    STATE s = START;
    do{
        byte = read(fd, &byteGetter, 1);
        if(byte == 0)
            continue;
        bytes++;
        printf("%u ", byteGetter);
        s = infoMachine(s, byteGetter, frame);
        if(s == START)
            return 0;
    }while(s != STOP);
    printf("bytes received da i frame: %d\n", bytes);
    printFrame(frame, bytes);

    int length = byteDestuffing(bytes, frame);

    printf("after destuffing %d\n", length);

    if(realloc(frame, length*sizeof(unsigned char)) == NULL){
        printf("Realloc failed \n");
    }

    return length;
}

void printFrame(unsigned char * frame, int size){
    for(int i = 0; i < size; i++){
        printf("%u ", frame[i]);    
    }
    printf("\n\n");
}
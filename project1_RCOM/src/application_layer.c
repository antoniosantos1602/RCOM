// Application layer protocol implementation

#include "application_layer.h"

void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{
    connectionParameters.sequence_number = 0;
    connectionParameters.baudRate = baudRate;
    connectionParameters.nRetransmissions = nTries;
    for(int i = 0; i < 50; i++){
        connectionParameters.serialPort[i] = serialPort[i];
    }
    connectionParameters.timeout = timeout;
    if(strcmp(role, "tx") == 0){
        connectionParameters.role = 0;
        for(int i = 0; i < strlen(filename); i++){
            file.fileName[i] = filename[i];
        }

        sendFile();
    }
    else{
        connectionParameters.role = 1;
        receiveFile();
    }
}

int buildDataPacket(unsigned char *packet, int sequenceNumber, unsigned char *data, int length)
{
    packet[0] = ICTRL;
    packet[1] = sequenceNumber; 
    packet[2] = length / 256;
    packet[3] = length % 256;

    for (int i = 0; i < length; i++)
        packet[i + 4] = data[i];

    return length + 4;
}

int buildControlPacket(unsigned char control, unsigned char *packet)
{
    packet[0] = control;
    packet[1] = 0;
    int length = 0;
    int fileSize = file.size;
    printf("file.size : %d\n", file.size);
    while (fileSize > 0)
    {
        int rest = fileSize % 256;
        int div = fileSize / 256;
        length++;
        for (unsigned int i = 2 + length; i > 3; i--)
            packet[i] = packet[i - 1];
        packet[3] = (unsigned char)rest;
        fileSize = div;
    }
    packet[2] = (unsigned char)length;
    memcpy(&packet[3], &file.size, length);
    packet[3 + length] = 1;
    packet[4 + length] = (unsigned char)(strlen(file.fileName) + 1);

    for (unsigned int j = 0; j < (strlen(file.fileName) + 1); j++)
    { 
        packet[5 + length + j] = file.fileName[j];
    }

    return 6 + length + strlen(file.fileName);
}

int parseDataPacket(unsigned char *packet, unsigned char *data, int sequenceNumber)
{
    if (packet[0] != ICTRL)
        return 0;
    if(sequenceNumber != (int)packet[1])
        return 0;

    int size_of_data = 256*(int)packet[2]+(int)packet[3];

    for (int i = 0; i < size_of_data; i++)
    {
        data[i] = packet[i + 4];
    }

    return 1;
}

int parseControlPacket(unsigned char *packet)
{
    if (packet[0] != ISTART && packet[0] != IEND)
        return 0;
    
    int sizeLength;
    if(packet[1] == 0){
        sizeLength = (int)packet[2];
        memcpy(&file.size, &packet[3], sizeLength);
    } 
    else 
        return 0;
    if(packet[3 + sizeLength] == 1){
        int sizeName = (int)packet[4+sizeLength];
        memcpy(&file.fileName, &packet[5+sizeLength], sizeName);
    } 
    else 
        return 0;

    return 1;
}

void sendFile(){
    file.file = fopen(file.fileName, "r");
    if(file.file == NULL)
        return;

    struct stat st;
    stat(file.fileName, &st);
    file.size = st.st_size;

    llopen(&connectionParameters);
    printf("connection good\n");

    unsigned char *ctrlPacket = (unsigned char*) malloc(MAX_PAYLOAD_SIZE*sizeof(unsigned char));
    int ctrlSize = buildControlPacket(2,ctrlPacket);
    printf("sending ctrl packet \n");

    if(llwrite(ctrlPacket, ctrlSize, &connectionParameters)==0){
        perror("couldnt send ctrl packet\n");
        exit(1);
    }

    printf("sent ctrl packet\n");
    printf("start sending file\n");

    int bytesRead = 0, bytesWritten = 0, sequenceNumber = 0, length = 0;
    while(bytesRead < file.size){
        if(bytesRead + MAX_PAYLOAD_SIZE < file.size)
            length = MAX_PAYLOAD_SIZE;
        else 
            length = file.size - bytesRead;

        unsigned char *dataPacket = (unsigned char*) malloc((length + 4)*sizeof(unsigned char));
        unsigned char *auxDataPacket = (unsigned char*) malloc((length + 4)*sizeof(unsigned char));

        if(fread(auxDataPacket, length, 1, file.file) != 1){
            perror("couldnt read the file\n");
            exit(1);
        }

        int size = buildDataPacket(dataPacket, sequenceNumber, auxDataPacket, length);
        bytesWritten = llwrite(dataPacket, size, &connectionParameters);
        bytesRead += bytesWritten - 4; //header has 4 bytes
        sequenceNumber++;
        free(dataPacket);
        free(auxDataPacket);
    }

    unsigned char *ctrlPacket2 = (unsigned char*) malloc(MAX_PAYLOAD_SIZE*sizeof(unsigned char));
    ctrlSize = buildControlPacket(3,ctrlPacket2);
    printf("sending ctrl packet \n");

    if(llwrite(ctrlPacket2, ctrlSize, &connectionParameters)==0){
        perror("couldnt send ctrl packet\n");
        exit(1);
    }

    printf("ctrl packet sent\n");

    if(llclose(connectionParameters) < 0){
        perror("couldnt close port\n");
        exit(1);
    }

    if(fclose(file.file) != 0){
        perror("couldnt close file\n");
        exit(1);
    }

    free(ctrlPacket);
    free(ctrlPacket2);
}

void receiveFile(){
    llopen(&connectionParameters);
    printf("connection good\n");

    unsigned char *ctrlPacket = (unsigned char*) malloc(MAX_PAYLOAD_SIZE*sizeof(unsigned char));
    printf("receiving ctrl packet \n");

    if(llread(ctrlPacket, &connectionParameters) == 0){
        perror("couldnt receive ctrl packet\n");
        exit(1);
    }

    printf("received ctrl packet\n");
    if(parseControlPacket(ctrlPacket) == 0){
        perror("incorrect ctrl packet\n");
        exit(1);
    }
    printf("start receiving file\n");
    file.file = fopen("penguin-received.gif", "w"); //substitute with file.filename
    if(file.file == NULL)
        return;
    printf("file size: %d\n",file.size);
    int bytesRead = 0, bytesWritten = 0, sequenceNumber = 0, length = 0;
    while(bytesRead < file.size){
        
        if(bytesRead + MAX_PAYLOAD_SIZE < file.size)
            length = MAX_PAYLOAD_SIZE;
        else 
            length = file.size - bytesRead;

        unsigned char *dataPacket = (unsigned char*) malloc((length + 4)*sizeof(unsigned char));
        unsigned char *auxDataPacket = (unsigned char*) malloc((length + 4)*sizeof(unsigned char));

        bytesWritten = llread(auxDataPacket, &connectionParameters);
        if(bytesWritten == 0)   
            continue;
        if(parseDataPacket(auxDataPacket, dataPacket, sequenceNumber) == 0)
            continue;

        if(fwrite(dataPacket, length, 1, file.file) != 1){
            perror("couldnt read the file\n");
            exit(1);
        }

        bytesRead += bytesWritten - 4; //header has 4 bytes
        sequenceNumber = (sequenceNumber + 1) % 256;
        free(dataPacket);
        free(auxDataPacket);
    }

    unsigned char *ctrlPacket2 = (unsigned char*) malloc(MAX_PAYLOAD_SIZE*sizeof(unsigned char));
    printf("receiving ctrl packet \n");

    if(llread(ctrlPacket2, &connectionParameters)==0){
        perror("couldnt send ctrl packet\n");
        exit(1);
    }

    printf("ctrl packet sent\n");

    if(parseControlPacket(ctrlPacket2) == 0){
        perror("incorrect ctrl packet\n");
        exit(1);
    }

    if(llclose(connectionParameters) == 0){
        perror("couldnt close port\n");
        exit(1);
    }

    if(fclose(file.file) != 0){
        perror("couldnt close file\n");
        exit(1);
    }

    free(ctrlPacket);
    free(ctrlPacket2);
}
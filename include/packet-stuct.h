#define HEADER_SIZE 8
#define PACKET_SIZE 1024


char* RRQ_WRQ_packet_create(int* returnSize,int opcode, char* filename, char* mode/*, char (*options)[]*/) { 
    size_t sizeOfPacket = 4+strlen(filename)+strlen(mode);
    char* packet = (char *) malloc(sizeOfPacket);

    if (1>opcode>9){
        fprintf (stdout, "Internal ERROR (wrong OPCODE)");
        exit(1);
    }
    if(filename==NULL){
        fprintf (stdout, "Internal ERROR (wrong FILENAME)");
        exit(1);
    }
    if(mode==NULL){
        fprintf (stdout, "Internal ERROR (wrong MODE)");
        exit(1);
    }

    char nullRepl[] = {32};
    strcpy(packet, nullRepl);
    packet[1] = opcode;
    strcat(packet, filename);
    strcat(packet, nullRepl);
    strcat(packet, mode);
    strcat(packet, nullRepl);

    printf("contents of char* array: ");
    for (int i = 0; i <= sizeOfPacket;i++){
        if(packet[i]== nullRepl[0]){
            packet[i] = (char)0;
        }
        printf("%i ", packet[i]);
    }

    *returnSize = sizeOfPacket;
    return packet;
}

int RRQ_WRQ_packet_read(char* packet, char* filename, char* mode){
    if(0>packet[1]>9){
        fprintf(stdout, "Client send a frond OPCODE of %d", packet[1]);
        return -1;
    }
    else{
        return packet[1];
    }
    return -1;
}
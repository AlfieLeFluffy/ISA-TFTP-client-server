#include "../include/error_code_msg.h"

char* ERR_packet_create(int* _returnSize, int _errorCode, char* _errorMessage) { 
    size_t sizeOfPacket = 4+strlen(_errorMessage);
    char* packet = (char *) malloc(sizeOfPacket);
    if(_errorMessage==NULL){
        fprintf (stdout, "ERROR: internal error (no error code message)");
        exit(1);
    }

    char nullRepl[] = {'*'};

    packet[0] = nullRepl[0];
    packet[1] = 5;
    packet[2] = nullRepl[0];
    packet[3] = _errorCode;
    strcat(packet, _errorMessage);
    strcat(packet, nullRepl);

    for (int i = 0; i <= sizeOfPacket;i++){
        if(packet[i]== nullRepl[0]){
            packet[i] = (char)0;
        }
    }

    *_returnSize = sizeOfPacket;
    return packet;
}

int ERR_packet_read(char* _packet, char* _errorMessage){
    if(_packet[1] != 5){
        fprintf(stdout, "ERROR: internal error (wrong opcode in error_packet_read)");
        return -1;
    }

    int index = 4;
    
    while (_packet[index] != 0){
        char charStr[2];
        charStr [0] = _packet[index];
        charStr[1] = '\0';
        strcat(_errorMessage, charStr);
        index++;
    }
    strcat(_errorMessage, "\0");

    return (int)_packet[3];
}

void ERR_message_write(char* _ip, int _sourcePort, int _destinPort, int _errorCode, char* _errorMessage){
    fprintf(stderr, "ERROR %s:%d:%d %d \"%s\"\n",_ip, _sourcePort, _destinPort, _errorCode, _errorMessage);
}

void ERR_packet_send(int listenfd, struct sockaddr_in* servaddr, struct sockaddr_in* cliaddr, int errorCode){
    int sizeOfPacket;
    char* errorPacket = ERR_packet_create(&sizeOfPacket, errorCode, errorMessage[errorCode]);
    sendto(listenfd, errorPacket, sizeOfPacket, 0,(struct sockaddr*)&cliaddr, sizeof(cliaddr));
    ERR_message_write(inet_ntoa(servaddr->sin_addr),servaddr->sin_port, cliaddr->sin_port,errorCode,errorMessage[errorCode]);
    free(errorPacket);
}
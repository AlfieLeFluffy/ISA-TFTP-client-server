///////////////////////////////////////////////////////////////////////////////////////////
///                                                                                     ///
///     TFTP server/client include                                                      ///
///                                                                                     ///
///     vytvoril: Tomas Vlach                                                           ///
///     login: xvlach24                                                                 ///
///                                                                                     ///
///////////////////////////////////////////////////////////////////////////////////////////

///
///     Includes all functions working with OACK packets
///

char* OACK_packet_create(int* _returnSize, int _blockSize, int _timeout, int _tsize) { 
    size_t sizeOfPacket = 2+strlen("blksize")+4+strlen("timeout")+3+strlen("tsize")+4;
    char* packet = (char *) malloc(sizeOfPacket);
    memset(packet, 0, sizeOfPacket);

    int index = 2; 

    packet[0] = '\0';
    packet[1] = 6;

    for(int i = 0; i< strlen("blksize");i++){
        packet[index+i]="blksize"[i];
    }
    index += strlen("blksize");
    packet[index] = '\0';
    index++;

    packet[index+1]=_blockSize; 
    _blockSize = _blockSize>>8; 
    packet[index]=_blockSize;
    index += 2;

    packet[index] = '\0';
    index++;

    for(int i = 0; i< strlen("timeout");i++){
        packet[index+i]="timeout"[i];
    }
    index += strlen("timeout");

    packet[index] = '\0';
    index++;

    packet[index]=_timeout;
    index++; 
    
    packet[index] = '\0';
    index++;

    for(int i = 0; i< strlen("tsize");i++){
        packet[index+i]="tsize"[i];
    }
    index += strlen("tsize");
    packet[index] = '\0';
    index++;

    packet[index+1]=_tsize; 
    _tsize = _tsize>>8; 
    packet[index]=_tsize;
    index += 2;
    
    packet[index] = '\0';
    index++;

    *_returnSize = sizeOfPacket;
    return packet;
}

int OACK_packet_read(char* _packet, int _packetLenght,unsigned int* _blockSize, unsigned int* _timeout, unsigned int* _tsize){
    if(_packet[1] != 6){
        fprintf(stdout, "ERROR: internal error (wrong opcode in error_packet_read)");
        return -1;
    }

    int index = 2;
    char option[25];

    while(index < _packetLenght){
        bzero(&option, sizeof(option));
        while(_packet[index] != 0){
            append_char(option, _packet[index]);
            index++;
        }
        strcat(option, "\0");
        index++;

        *option = tolower(*option);
        if(!strcmp(option, "blksize")){
            *_blockSize = (unsigned char)_packet[index] << 8 | (unsigned char) _packet[index+1];
            index += 3;
        }
        else if(!strcmp(option, "timeout")){
            *_timeout = (unsigned char)_packet[index];
            index += 2;
        }
        else if (!strcmp(option, "tsize")){
            *_tsize = (unsigned char)_packet[index] << 8 | (unsigned char) _packet[index+1];
            index += 3;
        }
        else{
            return -8;
        }

    }
    

    return 0;
}

void OACK_packet_write(char* _ip, int _sourcePort, int _blockSize, int _timeout, int _tsize){
    fprintf(stderr, "OACK %s:%d: blksize=%d timeout=%d tsize=%d\n",_ip, _sourcePort, _blockSize, _timeout, _tsize);
}

void OACK_packet_send(int _listenfd, struct sockaddr_in* _servaddr, struct sockaddr_in* _cliaddr, int _cliaddrSize, int _blockSize, int _timeout, int _tsize){
    int sizeOfPacket;
    char* oackPacket = OACK_packet_create(&sizeOfPacket, _blockSize, _timeout, _tsize);
    if(sendto(_listenfd, oackPacket, sizeOfPacket, 0,(struct sockaddr*)_cliaddr, _cliaddrSize) == -1){
        fprintf(stdout, "ERROR: ERROR packet, errno %d \n", errno);
    }
    free(oackPacket);
}
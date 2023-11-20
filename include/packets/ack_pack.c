char* ACK_packet_create(int* _returnSize, short _blockID) { 
    size_t sizeOfPacket = 4;
    char* packet = (char *) malloc(sizeOfPacket);

    packet[0] = '\0';
    packet[1] = 4;
    packet[2]=_blockID; 
    _blockID = _blockID>>8; 
    packet[3]=_blockID; 

    *_returnSize = sizeOfPacket;
    return packet;
}

int ACK_packet_read(char* _packet){
    if(_packet[1] != 4){
        fprintf(stdout, "ERROR: internal error (wrong opcode in ack_packet_read)\n");
        return -1;
    }
    

    return (short)(((short)_packet[2]) << 8) | _packet[3];
}

void ACK_message_write(char* _ip, int _sourcePort, int _blockID){
    fprintf(stderr, "ACK %s:%d %d \n",_ip, _sourcePort, _blockID);
}

void ACK_packet_send(int _listenfd, struct sockaddr_in* _servaddr, struct sockaddr_in* _cliaddr, int _cliaddrSize, int _blockID){
    int sizeOfPacket;
    char* ackPacket = ACK_packet_create(&sizeOfPacket, _blockID);
    if(sendto(_listenfd, ackPacket, sizeOfPacket, 0,(struct sockaddr*)_cliaddr, _cliaddrSize) == -1){
        fprintf(stdout, "ERROR: ERROR packet, errno %d \n", errno);
    }
    free(ackPacket);
}
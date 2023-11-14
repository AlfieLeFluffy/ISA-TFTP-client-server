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
        fprintf(stdout, "ERROR: internal error (wrong opcode in error_packet_read)");
        return -1;
    }
    

    return (short)(((short)_packet[2]) << 8) | _packet[3];
}

void ACK_message_write(char* _ip, int _sourcePort, int _blockID){
    fprintf(stderr, "ACK %s:%d: %d \n",_ip, _sourcePort, _blockID);
}
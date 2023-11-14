char* DATA_packet_create(int* _returnSize, int _blockID, char* _data) { 
    size_t sizeOfPacket = 4+sizeof(_data);
    char* packet = (char *) malloc(sizeOfPacket);

    packet[0] = '\0';
    packet[1] = 3;

    packet[2]=_blockID; 
    _blockID = _blockID>>8; 
    packet[3]=_blockID;
    

    *_returnSize = sizeOfPacket;
    return packet;
}

int DATA_packet_read(char* _packet, char* _filename, char* _mode){
    if(_packet[1] != 4){
        fprintf(stdout, "ERROR: internal error (wrong opcode in error_packet_read)");
        return -1;
    }

    return (short)(((short)_packet[2]) << 8) | _packet[3];
}

void DATA_request_write(int _opcode, struct sockaddr_in* _sinFrom, struct sockaddr_in* _sinTo, int _blockID){
    fprintf(stderr, "DATA %s:%d:%d %d\n", inet_ntoa(_sinFrom->sin_addr),_sinFrom->sin_port, _sinTo->sin_port, _blockID);  
}
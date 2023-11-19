char* DATA_packet_create(int* _returnSize, int _blockID, char* _data, int _sizeOfData) { 
    int sizeOfPacket = 4+_sizeOfData;

    char* packet = (char *) malloc(sizeOfPacket*sizeof(char));


    packet[0] = '\0';
    packet[1] = 3;

    packet[3]=_blockID; 
    _blockID = _blockID>>8; 
    packet[2]=_blockID;

    for(int i = 4; i< _sizeOfData+4;i++){
        packet[i]=_data[i-4];
    }
    

    *_returnSize = sizeOfPacket;
    return packet;
}

int DATA_packet_read(char* _packet, char* _filename, char* _mode){
    if(_packet[1] != 4){
        fprintf(stdout, "ERROR: internal error (wrong opcode in DATA_packet_read)");
        return -1;
    }

    return (short)(((short)_packet[2]) << 8) | _packet[3];
}

void DATA_request_write(int _opcode, struct sockaddr_in* _sinFrom, struct sockaddr_in* _sinTo, int _blockID){
    fprintf(stderr, "DATA %s:%d:%d %d\n", inet_ntoa(_sinFrom->sin_addr),_sinFrom->sin_port, _sinTo->sin_port, _blockID);  
}
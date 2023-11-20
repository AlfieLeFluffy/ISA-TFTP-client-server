///////////////////////////////////////////////////////////////////////////////////////////
///                                                                                     ///
///     TFTP server/client include                                                      ///
///                                                                                     ///
///     vytvoril: Tomas Vlach                                                           ///
///     login: xvlach24                                                                 ///
///                                                                                     ///
///////////////////////////////////////////////////////////////////////////////////////////

///
///     Includes all functions working with DATA packets
///


char* DATA_packet_create(int* _returnSize, int _blockID, char* _data, int _sizeOfData) { 
    int sizeOfPacket = 4+_sizeOfData;
    char* packet = (char *) malloc(sizeOfPacket*sizeof(char));
    memset(packet, 0, sizeOfPacket);


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

char* DATA_packet_read(char* _packet, int* _sizeOfData, int* _responceBlockID, char* _data, char* _mode, int _blockSize, int n){
    char* data = (char *) malloc(n*sizeof(char));
    memset(data, 0, n);

    if(_packet[1] != 3){
        fprintf(stdout, "ERROR: internal error (wrong opcode in DATA_packet_read)\n");
        *_responceBlockID = -1;
        return NULL;
    }

    for(int i = 4; i < n;i++){
        data[i-4]=_packet[i];
    }

    *_responceBlockID = (unsigned char)_packet[2] << 8 | (unsigned char) _packet[3];
    *_sizeOfData = n;
    return data;
}

void DATA_packet_write(char* _ip, int _sourcePort, int _destinPort, int _blockID){
    fprintf(stderr, "DATA %s:%d:%d %d\n", _ip, _sourcePort, _destinPort, _blockID);  
}
char* RRQ_WRQ_packet_create(int* _returnSize,int _opcode, char* _filename, char* _mode/*, char (*options)[]*/) { 
    size_t sizeOfPacket = 4+strlen(_filename)+strlen(_mode);
    char* packet = (char *) malloc(sizeOfPacket);

    if (_opcode != 1 && _opcode != 2){
        fprintf (stdout, "Internal ERROR (wrong OPCODE)");
        exit(1);
    }
    if(_filename==NULL){
        fprintf (stdout, "Internal ERROR (wrong FILENAME)");
        exit(1);
    }
    if(_mode==NULL){
        fprintf (stdout, "Internal ERROR (wrong MODE)");
        exit(1);
    }

    char nullRepl[] = {32};

    strcpy(packet, nullRepl);
    packet[1] = _opcode;
    strcat(packet, _filename);
    strcat(packet, nullRepl);
    strcat(packet, _mode);
    strcat(packet, nullRepl);

    for (int i = 0; i <= sizeOfPacket;i++){
        if(packet[i]== nullRepl[0]){
            packet[i] = (char)0;
        }
    }

    *_returnSize = sizeOfPacket;
    return packet;
}

int RRQ_WRQ_packet_read(char* _packet, char* _filename, char* _mode){
    if(_packet[1] != 1 && _packet[1] != 2){
        fprintf(stdout, "ERROR: Client send an incorrect OPCODE of %d\n", _packet[1]);
        return -1;
    }
    int index = 2;
    
    while (_packet[index] != 0){
        char charStr[2];
        charStr [0] = _packet[index];
        charStr[1] = '\0';
        strcat(_filename, charStr);
        index++;
    }
    strcat(_filename, "\0");

    index++;

    while(_packet[index] != 0){
        char charStr[2];
        charStr [0] = _packet[index];
        charStr[1] = '\0';
        strcat(_mode, charStr);
        index++;
    }
    strcat(_mode, "\0");

    return (int)_packet[1];
}

void RRQ_WRQ_request_write(int _opcode, struct sockaddr_in* _sin, char* _filePath, char* _mode){
    switch(_opcode){
        case 1:
                fprintf(stderr, "RRQ %s:%d \"%s\" %s {$OPTS}\n", inet_ntoa(_sin->sin_addr),_sin->sin_port, _filePath, _mode);
                break;
        case 2:
                fprintf(stderr, "WRQ %s:%d \"%s\" %s {$OPTS}\n", inet_ntoa(_sin->sin_addr),_sin->sin_port, _filePath, _mode);
                break;
    }
}
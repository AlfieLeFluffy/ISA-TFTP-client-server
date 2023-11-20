///////////////////////////////////////////////////////////////////////////////////////////
///                                                                                     ///
///     TFTP server/client include                                                      ///
///                                                                                     ///
///     vytvoril: Tomas Vlach                                                           ///
///     login: xvlach24                                                                 ///
///                                                                                     ///
///////////////////////////////////////////////////////////////////////////////////////////

///
///     Includes all functions working with RRQ/WRQ packets
///

/// @brief Creates char* array RRQ/WRQ packet
/// @param _returnSize 
/// @param _opcode 
/// @param _filename 
/// @param _mode 
/// @param _blockSize 
/// @param _timeout 
/// @param _tsize 
/// @return char* array packet
char* RRQ_WRQ_packet_create(int* _returnSize,int _opcode, char* _filename, char* _mode, int _blockSize, int _timeout, int _tsize) { 
    size_t sizeOfPacket = 4+strlen(_filename)+strlen(_mode)+strlen("blksize")+4+strlen("timeout")+3+strlen("tsize")+4;
    char* packet = (char *) malloc(sizeOfPacket);
    memset(packet, 0, sizeOfPacket);

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
    if(0>_timeout){
        fprintf (stdout, "Internal ERROR (wrong TIMEOUT)");
        exit(1);
    }
    if(0>_tsize){
        fprintf (stdout, "Internal ERROR (wrong TSIZE)");
        exit(1);
    }

    int index = 2;

    packet[0] = '\0';
    packet[1] = _opcode;

    for(int i = 0; i< strlen(_filename);i++){
        packet[index+i]=_filename[i];
    }
    index += strlen(_filename);

    packet[index+1] = '\0';
    index++;

    for(int i = 0; i< strlen(_mode);i++){
        packet[index+i]=_mode[i];
    }
    index += strlen(_mode);
    
    packet[index+1] = '\0';
    index++;

    for(int i = 0; i< strlen("blksize");i++){
        packet[index+i]="blksize"[i];
    }
    index += strlen("blksize");
    packet[index+1] = '\0';
    index++;

    packet[index+1]=_blockSize; 
    _blockSize = _blockSize>>8; 
    packet[index]=_blockSize;
    index += 2;

    packet[index+1] = '\0';
    index++;

    for(int i = 0; i< strlen("timeout");i++){
        packet[index+i]="timeout"[i];
    }
    index += strlen("timeout");

    packet[index+1] = '\0';
    index++;

    packet[index]=_timeout;
    index++; 
    
    packet[index+1] = '\0';
    index++;

    for(int i = 0; i< strlen("tsize");i++){
        packet[index+i]="tsize"[i];
    }
    index += strlen("tsize");
    packet[index+1] = '\0';
    index++;

    packet[index+1]=_tsize; 
    _tsize = _tsize>>8; 
    packet[index]=_tsize;
    index += 2;
    
    packet[index+1] = '\0';
    index++;


    *_returnSize = sizeOfPacket;
    return packet;
}

/// @brief Read a request packet and parses it into variables
/// @param _packet 
/// @param _packetLenght 
/// @param _filename 
/// @param _mode 
/// @param _blockSize 
/// @param _timeout 
/// @param _tsize 
/// @return 0 - OK, # - ERROR
int RRQ_WRQ_packet_read(char* _packet, int _packetLenght, char* _filename, char* _mode, unsigned int* _blockSize, unsigned int* _timeout, unsigned int* _tsize){
    char option[25];

    if(_packet[1] != 1 && _packet[1] != 2){
        fprintf(stdout, "ERROR: Client send an incorrect OPCODE of %d\n", _packet[1]);
        return -1;
    }
    int index = 2;
    
    while (_packet[index] != 0){
        append_char(_filename, _packet[index]);
        index++;
    }
    strcat(_filename, "\0");
    index++;

    while(_packet[index] != 0){
        append_char(_mode, _packet[index]);
        index++;
    }
    strcat(_mode, "\0");
    index++;

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
            *_timeout = (unsigned int)_packet[index];
            index += 2;
        }
        else if (!strcmp(option, "tsize")){
            /*unsigned char helpCharArray[8];
            long unsigned int helpInt = 0;
            int helpCounter = 0;
            index++;

            while(_packet[index] != '\0'){
                helpCharArray[helpCounter] = _packet[index];
                helpCounter++;
                index++;   
            }

            for(int i = 0; i<helpCounter; i++){
                helpInt += helpCharArray[i]*scale(helpCounter-i-1);
            }*/

            *_tsize = (unsigned char)_packet[index] << 8 | (unsigned char) _packet[index+1];//helpInt;
            index += 3; //8-helpCounter;
        }
        else{
            return -8;
        }
    }
    
    return (int)_packet[1];
}

/// @brief Writes out ACK packet onto stderr
/// @param _opcode 
/// @param _sin 
/// @param _filePath 
/// @param _mode 
/// @param blocksize 
/// @param timeout 
/// @param tsize 
void RRQ_WRQ_packet_write(int _opcode, struct sockaddr_in* _sin, char* _filePath, char* _mode, int blocksize, int timeout, int tsize){
    switch(_opcode){
        case 1:
                fprintf(stderr, "RRQ %s:%d \"%s\" %s blksize=%d timeout=%d tsize=%d\n", inet_ntoa(_sin->sin_addr),ntohs(_sin->sin_port), _filePath, _mode, blocksize,timeout,tsize);
                break;
        case 2:
                fprintf(stderr, "WRQ %s:%d \"%s\" %s blksize=%d timeout=%d tsize=%d\n", inet_ntoa(_sin->sin_addr),ntohs(_sin->sin_port), _filePath, _mode, blocksize,timeout,tsize);
                break;
    }
}
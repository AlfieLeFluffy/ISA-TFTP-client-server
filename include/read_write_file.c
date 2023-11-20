///////////////////////////////////////////////////////////////////////////////////////////
///                                                                                     ///
///     TFTP server/client include                                                      ///
///                                                                                     ///
///     vytvoril: Tomas Vlach                                                           ///
///     login: xvlach24                                                                 ///
///                                                                                     ///
///////////////////////////////////////////////////////////////////////////////////////////

char* read_file(FILE* _targetFile, char* _mode, int _blockSize, int* _sizeOfData){
    int index = 0;
    char c;

    char* outputData = (char *)malloc(_blockSize);
    memset(outputData, 0, _blockSize);

    while(index < _blockSize){
        c = fgetc(_targetFile);
        if(c == EOF) break;
        
        if((_mode == "netascii" || _mode == "mail" ) && c == '\n') {
            append_char(outputData,'\r');
            index++;
        }
        append_char(outputData, c);
        index++;
    }

    *_sizeOfData = index;
    return outputData;
}

int write_file(FILE* _targetFile, char* _buffer, char* _mode, int _sizeOfData){
    int index = 0;

    while(index < _sizeOfData){
        if((_mode == "netascii" || _mode == "mail" ) && _buffer[index] == '\r' && _buffer[index+1] == '\n') {
            fprintf(_targetFile,"\n");
            index++;
            index++;
        }
        fprintf(_targetFile, "%c", _buffer[index]);
        index++;
    }

    return 0;
}
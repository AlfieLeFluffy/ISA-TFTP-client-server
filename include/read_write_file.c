char* read_file(FILE* _targetFile, char* _mode, int _blockSize, int* _sizeOfData, int* _indexInFile){
    int charsRead = 0;
    char c;

    char* outputData = (char *)malloc(_blockSize);
    memset(outputData, 0, _blockSize);

    while(charsRead < _blockSize){
        c = fgetc(_targetFile);
        if(c == EOF) break;
        
        if(_mode == "netascii" && c == '\n') {
            append_char(outputData,'\r');
            charsRead++;
        }
        append_char(outputData, c);
        charsRead++;
    }

    *_sizeOfData = charsRead;
    return outputData;
}
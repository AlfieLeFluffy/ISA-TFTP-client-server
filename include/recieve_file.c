int recieve_file(int _listenfd, struct sockaddr_in* _servaddr, struct sockaddr_in* _cliaddr, int _cliaddrSize, char* _filePath, char* _mode, int _blockSize){
        int sizeOfPacket, sizeOfData, blockID, endOfFile, lastACK, indexInFile;
        FILE *targetFile;
        char* dataPacket;
        //char buffer[_blockSize];

        blockID = 0;
        sizeOfData = _blockSize;

        if((targetFile = fopen(_filePath, "w")) < 0){
                fprintf(stdout, "ERROR: internal errror, file could not be opened\n");
                close(_listenfd);
                exit(1);
        }

        

        return 0;
}

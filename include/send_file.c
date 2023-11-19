int send_file(int _listenfd, struct sockaddr_in* _servaddr, struct sockaddr_in* _cliaddr, int _len, char* _filePath, char* _mode,int _blockSize){
        int sizeOfPacket, sizeOfData, blockID, endOfFile, lastACK, indexInFile;
        FILE *targetFile;
        char* dataPacket;
        //char buffer[_blockSize];

        blockID = 0;
        sizeOfData = _blockSize;

        if((targetFile = fopen(_filePath, "r")) < 0){
                ERR_packet_send(_listenfd, _servaddr, _cliaddr, _len, 0);
        }

        while(sizeOfData==_blockSize){

                if(blockID>10){
                        fprintf(stdout, "ERROR: DATA, blockID overflow \n");
                        return -1;
                }

                char * data = read_file(targetFile, _mode, _blockSize, &sizeOfData, &indexInFile);
                dataPacket = DATA_packet_create(&sizeOfPacket, blockID, data , sizeOfData);
                if(sendto(_listenfd, dataPacket, sizeOfPacket, 0,(struct sockaddr*)_cliaddr, _len) < 0){
                        fprintf(stdout, "ERROR: DATA, failed to send, errno %d \n", errno);
                }
                printf("size %d\n", sizeOfPacket);
                free(data);
                free(dataPacket);
                blockID++;

                //int n = recvfrom(_listenfd, buffer, sizeof(buffer),0, (struct sockaddr*)&_servaddr,&_len);
        }
        

        return 0;
}



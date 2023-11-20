///////////////////////////////////////////////////////////////////////////////////////////
///                                                                                     ///
///     TFTP server/client include                                                      ///
///                                                                                     ///
///     vytvoril: Tomas Vlach                                                           ///
///     login: xvlach24                                                                 ///
///                                                                                     ///
///////////////////////////////////////////////////////////////////////////////////////////

/*int send_file(int _listenfd, struct sockaddr_in* _servaddr, struct sockaddr_in* _cliaddr, int _len, char* _filePath, char* _mode,int _blockSize){
        int n, sizeOfPacket, sizeOfData, blockID, responceBlockID, timeoutCounter, len;
        FILE *targetFile;
        char* dataPacket;
        char buffer[4+_blockSize];

        blockID = 1;
        n = 0;
        timeoutCounter = 0;
        sizeOfData = _blockSize;

        if((targetFile = fopen(_filePath, "r")) < 0){
                ERR_packet_send(_listenfd, _servaddr, _cliaddr, _len, 0);
        }

        while(sizeOfData==_blockSize){

                char* data = read_file(targetFile, _mode, _blockSize, &sizeOfData);
                dataPacket = DATA_packet_create(&sizeOfPacket, blockID, data , sizeOfData);

                // Send packet and resend if timeout is reached
                while(n<=0 && timeoutCounter < 3){
                        if(sendto(_listenfd, dataPacket, sizeOfPacket, 0,(struct sockaddr*)_cliaddr, _len) < 0){
                                fprintf(stdout, "ERROR: DATA, failed to send, errno %d \n", errno);
                        }
                        int len = _len;
                        n = recvfrom(_listenfd, buffer, sizeof(buffer),0, (struct sockaddr*)_cliaddr,&len);
                        if(n<0){
                                fprintf(stdout, "ERROR: DATA, timeout error %d \n", n);
                                timeoutCounter ++;
                        }
                }
                if(timeoutCounter >= 2){
                        ERR_packet_send(_listenfd, _servaddr, _cliaddr,_len,0);
                        fprintf(stdout,"ERROR: timeout counter reached, ending transmition\n");
                        close(_listenfd);
                        exit(1);
                }

                // Reset timeout counter
                timeoutCounter = 0;
                if(responceBlockID = ACK_packet_read(buffer) < 0){
                        ERR_packet_send(_listenfd, _servaddr, _cliaddr,_len,0);
                        fprintf(stdout,"ERROR: wrong opcode, ending transmition\n");
                        close(_listenfd);
                        exit(1);
                }

                if(blockID - 1 == responceBlockID){
                        while(n<=0 && timeoutCounter < 3){
                                if(sendto(_listenfd, dataPacket, sizeOfPacket, 0,(struct sockaddr*)_cliaddr, _len) < 0){
                                        fprintf(stdout, "ERROR: DATA, failed to send, errno %d \n", errno);
                                }

                                n = recvfrom(_listenfd, buffer, sizeof(buffer),0, (struct sockaddr*)_servaddr,&_len);
                                if(n<0){
                                        fprintf(stdout, "ERROR: DATA, timeout error %d \n", n);
                                        timeoutCounter ++;
                                }
                        }    
                }
                else if (blockID == responceBlockID){
                        free(data);
                        free(dataPacket);
                        blockID++;
                }
                else {
                        ERR_packet_send(_listenfd, _servaddr, _cliaddr,_len,5);
                        close(_listenfd);
                        exit(1);
                }
        }
        
        fclose(targetFile);
        close(_listenfd);
        return 0;
}*/



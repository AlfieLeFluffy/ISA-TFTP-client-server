///////////////////////////////////////////////////////////////////////////////////////////
///                                                                                     ///
///     TFTP server/client include                                                      ///
///                                                                                     ///
///     vytvoril: Tomas Vlach                                                           ///
///     login: xvlach24                                                                 ///
///                                                                                     ///
///////////////////////////////////////////////////////////////////////////////////////////

/*int recieve_file(int _sock, struct sockaddr_in* _servaddr, struct sockaddr_in* _cliaddr, int _len, char* _filePath, char* _mode, int _blockSize){
        int n,sizeOfPacket, sizeOfData, blockID, responceBlockID, timeoutCounter;
        FILE *targetFile;
        char* data;
        char buffer[4+_blockSize];

        n = 0;
        blockID = 1;
        sizeOfData = _blockSize;
        timeoutCounter = 0;

        if((targetFile = fopen(_filePath, "a")) < 0){ 
                fprintf(stdout, "ERROR: internal errror, file could not be opened\n");
                close(_sock);
                exit(1);
        }

        while (sizeOfData >= _blockSize){
                
                while(n<=0 && timeoutCounter < 3){
                n = recvfrom(_sock, &buffer, sizeof buffer,0, (struct sockaddr*)&_servaddr,&_len);
                        if(n<0){
                                fprintf(stdout, "ERROR: DATA, timeout error 1 %d errno %d", n, errno);
                                timeoutCounter ++;
                        }

                

                
                }
                if(timeoutCounter >= 2){
                        ERR_packet_send(_sock, _cliaddr,_servaddr,_len,0);
                        fprintf(stdout,"ERROR: timeout counter reached, ending transmition\n");
                        close(_sock);
                        exit(1);
                }
                // Reset timeout counter
                timeoutCounter = 0;
                data = DATA_packet_read(buffer, &sizeOfData, &responceBlockID,data,_mode,_blockSize,n);
                if(responceBlockID < 0 ){
                        ERR_packet_send(_sock, _cliaddr,_servaddr,_len,0);
                        fprintf(stdout,"ERROR: timeout counter reached, ending transmition\n");
                        close(_sock);
                        exit(1);
                }
                
                DATA_message_write(inet_ntoa(_servaddr->sin_addr),ntohs(_servaddr->sin_port), ntohs(_cliaddr->sin_port), blockID);

                if(blockID - 1 == responceBlockID){
                        while(n<=0 && timeoutCounter < 3){
                                        while(n<=0 && timeoutCounter < 3){
                                n = recvfrom(_listenfd, buffer, sizeof(buffer),0, (struct sockaddr*)&_servaddr,&_len);
                                if(n<0){
                                        fprintf(stdout, "ERROR: DATA, timeout error 2 %d \n", n);
                                        timeoutCounter ++;
                                }
                        }
                        if(timeoutCounter >= 2){
                                ERR_packet_send(_listenfd,_cliaddr,_servaddr,_len,0);
                                fprintf(stdout,"ERROR: timeout counter reached, ending transmition\n");
                                close(_listenfd);
                                exit(1);
                                }                     
                        }    
                }
                else if (blockID == responceBlockID){
                        write_file(targetFile,data,_mode,n);
                        printf("%d\n", _sock);
                        ACK_packet_send(_sock,_cliaddr,_servaddr,_len,blockID);
                        blockID++;
                }
                else {
                        ERR_packet_send(_sock, _cliaddr,_servaddr,_len,5);
                        free(data);
                        close(_sock);
                        exit(1);
                }
                n = 0;
                
                free(data);
        }

        fclose(targetFile);
        close(_sock);
        exit(1);
        return 0;
}*/

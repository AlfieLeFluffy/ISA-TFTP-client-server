///////////////////////////////////////////////////////////////////////////////////////////
///                                                                                     ///
///     TFTP server                                                                     ///
///                                                                                     ///
///     vytvoril: Tomas Vlach                                                           ///
///     login: xvlach24                                                                 ///
///                                                                                     ///
///////////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <strings.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/statvfs.h>


#include "../include/common.c"
#include "../include/parser.c"

#include "../include/packets/request_pack.c"
#include "../include/packets/ack_pack.c"
#include "../include/packets/oack_pack.c"
#include "../include/packets/data_pack.c"
#include "../include/packets/error_pack.c"

#include "../include/read_write_file.c"
#include "../include/send_file.c"
#include "../include/recieve_file.c"


int test_TFTP_request(int _opcode, char* _filePath, char* _mode){
        if(_opcode != 1 && _opcode != 2){
                fprintf(stdout, "ERROR: opcode for request does not match possible opcodes, %d\n", _opcode);
                return 4;
        }

        switch(_opcode){
                case 1:
                        if (access(_filePath, F_OK) != 0) return 1;
                        break;
                case 2:
                        if (access(_filePath, F_OK) == 0) return 6;
                        break;
                default:
                        fprintf(stdout, "ERROR: Internal error\n");
                        return 1;
                        break;
        }
        return 0;
}

int handle_options(int _opcode, char* _folderpath, char* _filepath, int* _blockSize, int* _timeout, int* _tsize){
        FILE* testfile;
        switch(_opcode){
                case 1:
                        // Find size of file
                        testfile = fopen(_filepath, "r");
                        if (testfile == NULL) { 
                                printf("ERROR: Could not open file\n"); 
                                return 1; 
                        } 
                        fseek(testfile, 0L, SEEK_END); 
                        long int size = ftell(testfile); 
                        fclose(testfile); 

                        // Set blocksize for better file transfer dependent on size
                        if(size > 65500){
                                *_blockSize=65500;
                        }
                        else{
                                double amount = size/512;
                                amount = (int)(amount < 0 ? (amount - 0.5) : (amount + 0.5));
                                *_blockSize = 512 * (amount+1);
                        }

                        // Check timeout
                        if(0>*_timeout>256)*_timeout=1;

                        // Set tsize to filesize
                        *_tsize = size;

                        break;
                case 2:
                        // Check if blocksize is not outside allowed parameters
                        if(512>*_blockSize || *_blockSize>65500)*_blockSize=65500;

                        // Check timeout
                        if(0>*_timeout|| *_timeout>256)*_timeout=1;

                        // Set tsize to filesize
                        struct statvfs diskScanData;
                        if((statvfs(_folderpath,&diskScanData)) < 0 ) {
                                fprintf(stdout, "ERROR: unable to get free disk space\n");
                                return 0;
                        }

                        if(*_tsize>diskScanData.f_bfree){
                                fprintf(stdout, "ERROR: not enough free disk\n");
                                return 3;
                        }

                        break;
        }
        return 0;
}

///Joins incoming filename with default filefolder to create filepath
///Parameters are a char array of incoming filename and a char array of the default folderpath
char* create_file_path(char* _filename, char* _folderPath){
        char* filePath = (char*)malloc(sizeof(_filename)+sizeof(_folderPath)+2);
        
        strcat(filePath, _folderPath);
        if(_filename[0] != '/' && _folderPath[strlen(_folderPath)-1] != '/') strcat(filePath, "/");
        strcat(filePath, _filename);
        strcat(filePath, "\0");
        return filePath;
}

  
int main(int argc, char *argv[]) 
{
        int port = 69;
        int index;
        int c;
        char* folderPath;
        opterr = 0;

        
        while ((c = getopt (argc, argv, "p:")) != -1)
                switch (c){
                        case 'p':
                                port = atoi(optarg);
                                break;
                        case '?':
                                if (isprint (optopt)){
                                        fprintf (stdout, "Unknown option `-%c'.\n", optopt);
                                        exit(1);
                                }
                                        
                                else{
                                        fprintf (stdout, "Unknown option character `\\x%x'.\n", optopt);
                                        exit(1);
                                }
                                break;
                        default:
                                abort ();  
                                break;     
        }

        if(optind+1 < argc){
                fprintf(stdout, "Folder path is in an incorrect format\n");
                exit(1);
        }
        else{
                folderPath = parseFolderPath(argv[optind]);
        }

        char buffer[100];

        for (int i = 0; i<100;i++){
                buffer[i] = '\0';
        }       

        
        int listenfd, len;
        struct sockaddr_in servaddr,  cliaddr;

        bzero(&servaddr, sizeof(servaddr));

        // Create a UDP Socket
        listenfd = socket(AF_INET, SOCK_DGRAM, 0);        
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        servaddr.sin_port = htons(port);
        servaddr.sin_family = AF_INET; 

        // bind server address to socket descriptor
        if(bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr))){
                fprintf(stdout, "ERROR: creating socket, could not bind\n");
                exit(1);
        }

        while(1){
                //receive the datagram
                len = sizeof(cliaddr);
                int n = recvfrom(listenfd, buffer, sizeof(buffer),0, (struct sockaddr*)&cliaddr,&len); //receive message from server


                while(!fork()){

                        struct sockaddr_in *clientaddr_in = (struct sockaddr_in *)&cliaddr;
                        int opcode, sizeOfPacket, blockID, blockSize, timeout, tsize, errorCode;
                        char* filePath;
                        char filename[n], mode[50];
                        struct timeval timeout_struct; 

                        blockSize = 512;
                        timeout = 1;
                        tsize = 0;

                        bzero(&filename, sizeof(filename));
                        bzero(&mode, sizeof(mode));
                        bzero(&servaddr, sizeof(servaddr));

                        // Create a UDP Socket
                        listenfd = socket(AF_INET, SOCK_DGRAM, 0);        
                        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
                        servaddr.sin_port = htons(1025);
                        servaddr.sin_family = AF_INET; 

                        // bind client address to socket descriptor
                        while (bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr))){
                                if(servaddr.sin_port >= 65535){
                                printf("ERROR : could not bind socket in return mode, %d \n", errno);
                                exit(1);
                                }
                                servaddr.sin_port = htons(servaddr.sin_port +1);
                        }

                        // connect to server
                        while(connect(listenfd, (struct sockaddr *)&cliaddr, sizeof(cliaddr)) < 0)
                        {
                                printf("ERROR : could not connect, %d \n", errno);
                                error_exit_FD(1, listenfd);
                        }

                        opcode = RRQ_WRQ_packet_read(buffer, n, filename, mode, &blockSize, &timeout, &tsize);
                        if(opcode < 0){
                                ERR_packet_send(listenfd, &servaddr, &cliaddr, sizeof(cliaddr), opcode*(-1));
                                close(listenfd);
                                return -1;
                        }

                        
                        filePath = create_file_path(filename, folderPath);
                        RRQ_WRQ_packet_write(opcode, clientaddr_in, filePath, mode, blockSize, timeout, tsize);
                        
                        errorCode = handle_options(opcode, folderPath, filePath, &blockSize, &timeout, &tsize);
                        if(errorCode>0){
                                ERR_packet_send(listenfd, &servaddr, &cliaddr, sizeof(cliaddr), errorCode);
                                close(listenfd);
                                return -1;
                        }
                        else if(errorCode = test_TFTP_request(opcode, filePath, mode)){
                                ERR_packet_send(listenfd, &servaddr,&cliaddr, sizeof(cliaddr),errorCode);
                                return -1;
                        }

                        // Set socket timeout     
                        timeout_struct.tv_sec = timeout;
                        timeout_struct.tv_usec = 0;
                        if (setsockopt (listenfd, SOL_SOCKET, SO_RCVTIMEO, &timeout_struct,sizeof timeout_struct) < 0) fprintf(stdout,"ERROR : setsocketopt failed, timeout \n");        

                        OACK_packet_send(listenfd,&servaddr,&cliaddr,sizeof cliaddr, blockSize,timeout,tsize);

                        
                        int sizeOfData, responceBlockID, timeoutCounter;
                        char buffer2[4+blockSize];
                        FILE *readFromFile;
                        char* data;

                        
                        char errorMessage[sizeof(buffer)];
                        bzero(&errorMessage, sizeof(errorMessage));

                        // Switch and handle the file transfer 
                        switch(opcode){
                                case 1:
                                        n = recvfrom(listenfd, buffer2, sizeof(buffer2),0, (struct sockaddr*)&cliaddr,&len);
                                        if(ACK_packet_read(buffer2) != 0){
                                                ERR_packet_send(listenfd, &servaddr,&cliaddr,sizeof cliaddr,0);
                                                close(listenfd);
                                                exit(1);
                                        }
                                        ACK_packet_write(inet_ntoa(clientaddr_in->sin_addr),ntohs(clientaddr_in->sin_port),0);

                                        char* dataPacket;

                                        blockID = 1;
                                        n = 0;
                                        timeoutCounter = 0;
                                        sizeOfData = blockSize;
                                        len = sizeof servaddr;

                                        if((readFromFile = fopen(filePath, "r")) < 0){
                                                ERR_packet_send(listenfd, &servaddr, &cliaddr, len, 0);
                                        }

                                        while(sizeOfData>=blockSize){
                                                char* data = read_file(readFromFile, mode, blockSize, &sizeOfData);
                                                dataPacket = DATA_packet_create(&sizeOfPacket, blockID, data , sizeOfData);

                                                // Send packet and resend if timeout is reached
                                                while(n<=0 && timeoutCounter < 3){
                                                        if(sendto(listenfd, dataPacket, sizeOfPacket, 0,(struct sockaddr*)&cliaddr, len) < 0){
                                                                fprintf(stdout, "ERROR: DATA, failed to send, errno %d \n", errno);
                                                        }
                                                        n = recvfrom(listenfd, buffer2, sizeof(buffer2),0, (struct sockaddr*)&cliaddr,&len);
                                                        if(n<0){
                                                                fprintf(stdout, "ERROR: DATA, timeout error %d \n", n);
                                                                timeoutCounter ++;
                                                        }
                                                }
                                                if(timeoutCounter >= 2){
                                                        ERR_packet_send(listenfd, &servaddr, &cliaddr,len,0);
                                                        fprintf(stdout,"ERROR: timeout counter reached, ending transmition\n");
                                                        close(listenfd);
                                                        exit(1);
                                                }

                                                // Reset timeout counter
                                                timeoutCounter = 0;

                                                responceBlockID = ACK_packet_read(buffer2);
                                                if( responceBlockID < 0){
                                                        ERR_packet_send(listenfd, &servaddr, &cliaddr,len,0);
                                                        fprintf(stdout,"ERROR: wrong opcode, ending transmition\n ");
                                                        close(listenfd);
                                                        exit(1);
                                                }

                                                if(blockID > responceBlockID){
                                                        while(n<=0 && timeoutCounter < 3){
                                                                if(sendto(listenfd, dataPacket, sizeOfPacket, 0,(struct sockaddr*)&cliaddr, len) < 0){
                                                                        fprintf(stdout, "ERROR: DATA, failed to send, errno %d \n", errno);
                                                                }

                                                                n = recvfrom(listenfd, buffer2, sizeof(buffer2),0, (struct sockaddr*)&cliaddr,&len);
                                                                if(n<0){
                                                                        fprintf(stdout, "ERROR: DATA, timeout error %d \n ", n);
                                                                        timeoutCounter ++;
                                                                }
                                                        } 
                                                        if(timeoutCounter >= 2){
                                                                ERR_packet_send(listenfd, &servaddr, &cliaddr,len,0);
                                                                fprintf(stdout,"ERROR: timeout counter reached, ending transmition\n");
                                                                close(listenfd);
                                                                exit(1);
                                                        }   
                                                }
                                                else if (blockID == responceBlockID){
                                                        ACK_packet_write(inet_ntoa(clientaddr_in->sin_addr),ntohs(clientaddr_in->sin_port),responceBlockID);
                                                        free(data);
                                                        free(dataPacket);
                                                        blockID++;
                                                }
                                                else {
                                                        ERR_packet_send(listenfd, &servaddr, &cliaddr,len,5);
                                                        close(listenfd);
                                                        exit(1);
                                                }
                                                n = 0;
                                        }
                                        
                                        fclose(readFromFile);
                                        close(listenfd);

                                        //send_file(listenfd, &servaddr, &cliaddr, sizeof(cliaddr), filePath, mode, blockSize);
                                        break;
                                case 2:
                                        //recieve_file(listenfd, &servaddr, &cliaddr, sizeof(cliaddr), filePath, mode, blockSize);

                                        n = 0;
                                        blockID = 1;
                                        sizeOfData = blockSize;
                                        timeoutCounter = 0;
                                        len = sizeof cliaddr;

                                        if((readFromFile = fopen(filePath, "w")) < 0){
                                                fprintf(stdout, "ERROR: internal errror, file could not be opened\n");
                                                close(listenfd);
                                                exit(1);
                                        }

                                        while (sizeOfData >= blockSize){
                                                while(n<=0 && timeoutCounter < 3){
                                                        n = recvfrom(listenfd, buffer2, sizeof buffer2,0, (struct sockaddr*)&cliaddr,&len);
                                                        if(n<0){
                                                                fprintf(stdout, "ERROR: DATA, timeout error %d errno %d\n", n, errno);
                                                                timeoutCounter ++;
                                                        }
                                                }
                                                if(timeoutCounter >= 2){
                                                        ERR_packet_send(listenfd, &servaddr,&cliaddr,len,0);
                                                        fprintf(stdout,"ERROR: timeout counter reached, ending transmition\n");
                                                        close(listenfd);
                                                        exit(1);
                                                }
                                                // Reset timeout counter
                                                timeoutCounter = 0;
                                                data = DATA_packet_read(buffer2, &sizeOfData ,&responceBlockID,data,mode,blockSize,n);
                                                if(responceBlockID < 0 ){
                                                        errorCode = ERR_packet_read(buffer2, errorMessage);
                                                        ERR_packet_write(inet_ntoa(clientaddr_in->sin_addr),ntohs(clientaddr_in->sin_port), ntohs(servaddr.sin_port),errorCode,errorMessage);
                                                        close(listenfd);
                                                        exit(1);
                                                }
                                                
                                                DATA_packet_write(inet_ntoa(clientaddr_in->sin_addr),ntohs(clientaddr_in->sin_port), ntohs(servaddr.sin_port), blockID);

                                                if(blockID - 1 == responceBlockID){
                                                        while(n<=0 && timeoutCounter < 3){
                                                                        while(n<=0 && timeoutCounter < 3){
                                                                n = recvfrom(listenfd, buffer2, sizeof(buffer2),0, (struct sockaddr*)&cliaddr,&len);
                                                                if(n<0){
                                                                        fprintf(stdout, "ERROR: DATA, timeout error 2 %d \n", n);
                                                                        timeoutCounter ++;
                                                                }
                                                        }
                                                        if(timeoutCounter >= 2){
                                                                ERR_packet_send(listenfd,&servaddr,&cliaddr,len,0);
                                                                fprintf(stdout,"ERROR: timeout counter reached, ending transmition\n");
                                                                close(listenfd);
                                                                exit(1);
                                                                }                     
                                                        } 
                                                        if(responceBlockID < 0 ){
                                                                errorCode = ERR_packet_read(buffer2, errorMessage);
                                                                ERR_packet_write(inet_ntoa(clientaddr_in->sin_addr),ntohs(clientaddr_in->sin_port), ntohs(servaddr.sin_port),errorCode,errorMessage);
                                                                close(listenfd);
                                                                exit(1);
                                                        }   
                                                }
                                                else if (blockID == responceBlockID){
                                                        write_file(readFromFile,data,mode,n);
                                                        ACK_packet_send(listenfd,&servaddr,&cliaddr,sizeof cliaddr,blockID);
                                                        blockID++;
                                                }
                                                else {
                                                        ERR_packet_send(listenfd, &servaddr,&cliaddr,len,5);
                                                        free(data);
                                                        close(listenfd);
                                                        exit(1);
                                                }
                                                n = 0;
                                                free(data);
                                        }

                                        fclose(readFromFile);
                                        close(listenfd);


                                        
                                        break;
                        }

                        return 0;

                }
        }
        return 0;
}
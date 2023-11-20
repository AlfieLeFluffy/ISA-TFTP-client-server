///////////////////////////////////////////////////////////////////////////////////////////
///                                                                                     ///
///     TFTP client                                                                     ///
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
#include <netdb.h>
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


int check_ip_valid(char* _ip){
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, _ip, &(sa.sin_addr));
    if(result!=0)return 1;
    return 0;
}

char* resolve_hostname(char* _result,char* _hostname){
    struct addrinfo *result;
    if (getaddrinfo (_hostname, NULL, NULL, &result))
    {
        fprintf(stdout,"ERROR: cannot resolve hostname\n");
        return "\0";
    }
    return inet_ntoa(((struct sockaddr_in *)result->ai_addr)->sin_addr);
}

int handle_options_send(char* _filepath, unsigned int* _blockSize, unsigned int* _timeout, unsigned int* _tsize){
        FILE* testfile;

        // Find size of file
        testfile = fopen(_filepath, "r");
        if (testfile == NULL) { 
                printf("ERROR: Could not open file: option send\n"); 
                return 1; 
        } 
        fseek(testfile, 0L, SEEK_END); 
        long int size = ftell(testfile); 
        fclose(testfile); 

        if( size > 17196646400){
            printf("ERROR: File too big to be send over TFTP\n"); 
            return 1;
        }

        // Set blocksize for better file transfer dependent on size
        if(size > 65500){
                *_blockSize=65500;
        }
        else if(size < 512){
                *_blockSize = 512;
        }
        else{
            *_blockSize = 512 * (size%512);
        }

        // Check timeout
        if(0>*_timeout || *_timeout>256)*_timeout=1;

        // Set tsize to filesize
        *_tsize = size;
                        
        return 0;
}

int handle_options_recieve(int _blockSize, int _timeout, int _tsize){
        // Check if blocksize is not outside allowed parameters
        if(512>_blockSize || _blockSize>65500)return 8;

        // Check timeout
        if(0>_timeout || _timeout>256) return 1;

        // Set tsize to filesize
        struct statvfs diskScanData;
        if((statvfs(".",&diskScanData)) < 0 ) {
                return 1;
        }

        if(_tsize>diskScanData.f_bfree){
                return 3;
        }
    return 0;
}
  

int main(int argc, char *argv[]) 
{
    ////////////////////////////////////
    ///
    ///     START OF VARIABLE DEF


    char *ip = NULL;
    int port = 69;
    char *filePathDownload = NULL;
    char *filePathUpload = NULL;
    char *fileTargetPath = NULL;
    int c;
    opterr = 0;

    ///     END OF VARIABLE DEF
    ///
    ////////////////////////////////////
    ///
    ///     START OF PARAM PARSE AND CHECK


    while ((c = getopt (argc, argv, "h:p:f:t:")) != -1){
        switch (c){
            case 'h':
                ip = optarg;
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 'f':
                filePathDownload = optarg;
                break;
            case 't':
                fileTargetPath = optarg;
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
    }

    /*
        Checks for argument input combinations
    */
    if(optind+1 < argc){
        fprintf(stdout, "ERROR: File path is in an incorrect format\n");
        exit(1);
    }
    else if (optind < argc){
        filePathUpload = parseUploadFilePath(argv[optind]);
    }                       

    if (filePathDownload != NULL && filePathUpload != NULL){
        fprintf(stdout, "ERROR: Can't both download and upload in the same command\n");
        exit(1);
    }

    if(filePathDownload != NULL){
        fileTargetPath = parseFDownloadilePath(fileTargetPath);
    }

    if (ip == NULL){
        fprintf(stdout, "ERROR: -h is a required argument (IP of server)\n");
        exit(1);
    }

    // Checks if ip address is valid and tries to resolve for hostname if not
    if(!check_ip_valid(ip)){
        ip = resolve_hostname(ip,ip);
        if(strcmp(ip, "\0"))exit(1);
    }

    ///     END OF PARAM PARSE AND CHECK
    ///
    ////////////////////////////////////
    ///
    ///     START OF NETWORK SETUP


    // Definition of variables used
    char buffer[512];
    int sockfd, sizeOfPacket, opcode, blockID, errorCode;
    unsigned int n, blockSize, timeout, tsize;
    char* requestPacket;
    char* filePath;
    char* mode;
    struct sockaddr_in servaddr, cliaddr;
    struct timeval timeout_struct;
        
    // Clear and set server servaddr
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_addr.s_addr = inet_addr(ip);
    servaddr.sin_port = htons(port);
    servaddr.sin_family = AF_INET;

    // Clear and set client cliaddr
    bzero(&cliaddr, sizeof(cliaddr));
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);        
    cliaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    cliaddr.sin_port = htons(2048);
    cliaddr.sin_family = AF_INET; 

    // Bind client address to socket descriptor
    while (bind(sockfd, (struct sockaddr*)&cliaddr, sizeof(cliaddr))){
        if(cliaddr.sin_port >= 65535){
            printf("ERROR : could not bind socket \n");
            exit(1);
        }
        cliaddr.sin_port = htons(cliaddr.sin_port +1);
    }

    // Set socket timeout  
    timeout_struct.tv_sec = 1;
    timeout_struct.tv_usec = 0;
    if (setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout_struct,sizeof timeout_struct) < 0) fprintf(stdout,"ERROR : setsocketopt failed, timeout \n");
    
    
    // Connect to server
    while(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        printf("ERROR : connect Failed \n");
        error_exit_FD(1, sockfd);
    }

    ///     END OF NETWORK SETUP
    ///
    ////////////////////////////////////
    ///
    ///     START OF REQUEST

    // Create packet dependent on which opetation is required 
    mode = "netascii";
    if (filePathDownload != NULL){
        opcode = 1;
        filePath = filePathDownload;
        requestPacket = RRQ_WRQ_packet_create(&sizeOfPacket,1,filePathDownload,mode,65536,1,0);
    }
    else{
        opcode = 2;
        filePath = filePathUpload;
        handle_options_send(filePath, &blockSize, &timeout, &tsize);
        requestPacket = RRQ_WRQ_packet_create(&sizeOfPacket,2,fileTargetPath,mode,blockSize,timeout,tsize); 
    }
    sendto(sockfd, requestPacket, sizeOfPacket, 0, (struct sockaddr*)NULL, sizeof(servaddr));
    free(requestPacket);
    close(sockfd);

    ///     END OF REQUEST
    ///
    ////////////////////////////////////
    ///
    ///     START OF PARSING RESPONSE AND OACK

    // Reset cliaddr for inversed comunication
    bzero(&cliaddr, sizeof(cliaddr));
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);        
    cliaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    cliaddr.sin_port = htons(2048);
    cliaddr.sin_family = AF_INET; 

    // Bind client address to socket descriptor
    while (bind(sockfd, (struct sockaddr*)&cliaddr, sizeof(cliaddr))){
        printf("ERROR : could not bind socket \n");
        exit(1);
    }

    // Set socket timeout  
    timeout_struct.tv_sec = 1;
    timeout_struct.tv_usec = 0;
    if (setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout_struct,sizeof timeout_struct) < 0) fprintf(stdout,"ERROR : setsocketopt failed, timeout \n");
    
    

    // Receive responce from server
    unsigned int len = sizeof(servaddr);
    n = recvfrom(sockfd, buffer, sizeof(buffer),0, (struct sockaddr*)&servaddr,&len); 

    if(n<0){
        fprintf(stdout, "ERROR: ACK/OACK, timeout error %d \n", n);
        close(sockfd);
        exit(1);
    }

    char errorMessage[sizeof(buffer)];
    bzero(&errorMessage, sizeof(errorMessage));
    
    switch (buffer[1]){
        case 4:
            blockID = ACK_packet_read(buffer);
            ACK_packet_write(inet_ntoa(servaddr.sin_addr),ntohs(servaddr.sin_port), blockID);
            break;
        case 5:
            errorCode = ERR_packet_read(buffer, errorMessage);
            ERR_packet_write(inet_ntoa(servaddr.sin_addr),ntohs(servaddr.sin_port), ntohs(cliaddr.sin_port),errorCode,errorMessage);
            error_exit_FD(errorCode, sockfd);
            break;
        case 6:
            errorCode = OACK_packet_read(buffer,n,&blockSize,&timeout,&tsize);
            if(errorCode){
                fprintf(stdout, "ERROR: handling OACK responce: %d\n", errorCode);
                close(sockfd);
                exit(1);
            }
            OACK_packet_write(inet_ntoa(servaddr.sin_addr),ntohs(servaddr.sin_port),blockSize,timeout,tsize);
            errorCode = handle_options_recieve(blockSize, timeout, tsize);
            if(errorCode == 1){
                fprintf(stdout, "ERROR: handling OACK responce: unable to process request\n");
                close(sockfd);
                exit(1);
            }
            else if(errorCode == 3){
                fprintf(stdout, "ERROR: handling OACK responce: not enough available disk space\n");
                close(sockfd);
                exit(1);
            }
            break;
        default:
            fprintf(stdout, "ERROR: internal error (wrong answer OPCODE)\n");
            error_exit_FD(1,sockfd);
            break;
    }

    ///     END OF PARSING RESPONSE AND OACK
    ///
    ////////////////////////////////////
    ///
    ///     START OF START OF DOWNLOAD/UPLOAD LOGIC


    char buffer2[4+blockSize];
    char* dataPacket;
    int  sizeOfData, responceBlockID, timeoutCounter;
    FILE *readFromFile;
    
    switch(opcode){
            case 1:
                    ACK_packet_send(sockfd, &cliaddr, &servaddr, sizeof cliaddr, 0);

                    char* data;

                    n = 0;
                    blockID = 1;
                    sizeOfData = blockSize;
                    timeoutCounter = 0;
                    len = sizeof cliaddr;

                    if((readFromFile = fopen(fileTargetPath, "w")) < 0){
                            fprintf(stdout, "ERROR: internal errror, file could not be opened\n");
                            close(sockfd);
                            exit(1);
                    }

                    while (sizeOfData >= blockSize){
                            while(n<=0 && timeoutCounter < 3){
                                    n = recvfrom(sockfd, buffer2, sizeof buffer2,0, (struct sockaddr*)&servaddr,&len);
                                    if(n<0){
                                            fprintf(stdout, "ERROR: DATA, timeout error %d errno %d\n", n, errno);
                                            timeoutCounter ++;
                                    }
                            }
                            if(timeoutCounter >= 2){
                                    ERR_packet_send(sockfd, &cliaddr,&servaddr,len,0);
                                    fprintf(stdout,"ERROR: timeout counter reached, ending transmition\n");
                                    close(sockfd);
                                    exit(1);
                            }
                            // Reset timeout counter
                            timeoutCounter = 0;
                            data = DATA_packet_read(buffer2, &sizeOfData ,&responceBlockID,data,mode,blockSize,n);
                            if(responceBlockID < 0 ){
                                    errorCode = ERR_packet_read(buffer2, errorMessage);
                                    ERR_packet_write(inet_ntoa(servaddr.sin_addr),ntohs(servaddr.sin_port), ntohs(cliaddr.sin_port),errorCode,errorMessage);
                                    close(sockfd);
                                    exit(1);
                            }
                            
                            DATA_packet_write(inet_ntoa(servaddr.sin_addr),ntohs(servaddr.sin_port), ntohs(cliaddr.sin_port), blockID);

                            if(blockID - 1 == responceBlockID){
                                    while(n<=0 && timeoutCounter < 3){
                                                    while(n<=0 && timeoutCounter < 3){
                                            n = recvfrom(sockfd, buffer2, sizeof(buffer2),0, (struct sockaddr*)&servaddr,&len);
                                            if(n<0){
                                                    fprintf(stdout, "ERROR: DATA, timeout error 2 %d \n", n);
                                                    timeoutCounter ++;
                                            }
                                    }
                                    if(timeoutCounter >= 2){
                                            ERR_packet_send(sockfd,&cliaddr,&servaddr,len,0);
                                            fprintf(stdout,"ERROR: timeout counter reached, ending transmition\n");
                                            close(sockfd);
                                            exit(1);
                                            }                     
                                    } 
                                    if(responceBlockID < 0 ){
                                            errorCode = ERR_packet_read(buffer2, errorMessage);
                                            ERR_packet_write(inet_ntoa(servaddr.sin_addr),ntohs(servaddr.sin_port), ntohs(cliaddr.sin_port),errorCode,errorMessage);
                                            close(sockfd);
                                            exit(1);
                                    }   
                            }
                            else if (blockID == responceBlockID){
                                    write_file(readFromFile,data,mode,n);
                                    ACK_packet_send(sockfd,&cliaddr,&servaddr,sizeof cliaddr,blockID);
                                    blockID++;
                            }
                            else {
                                    ERR_packet_send(sockfd, &cliaddr,&servaddr,len,5);
                                    free(data);
                                    close(sockfd);
                                    exit(1);
                            }
                            n = 0;
                            free(data);
                    }

                    fclose(readFromFile);
                    close(sockfd);

                    break;
            case 2:
                    //send_file(sockfd, &servaddr, &cliaddr, sizeof(cliaddr), filePath, mode, blockSize);


                    blockID = 1;
                    n = 0;
                    timeoutCounter = 0;
                    sizeOfData = blockSize;
                    len = sizeof servaddr;

                    if((readFromFile = fopen(filePath, "r")) < 0){
                            ERR_packet_send(sockfd, &servaddr, &cliaddr, len, 0);
                    }

                    while(sizeOfData>=blockSize){
                            char* data = read_file(readFromFile, mode, blockSize, &sizeOfData);
                            dataPacket = DATA_packet_create(&sizeOfPacket, blockID, data , sizeOfData);

                            // Send packet and resend if timeout is reached
                            while(n<=0 && timeoutCounter < 3){
                                    if(sendto(sockfd, dataPacket, sizeOfPacket, 0,(struct sockaddr*)&servaddr, len) < 0){
                                            fprintf(stdout, "ERROR: DATA, failed to send, errno %d \n", errno);
                                    }
                                    n = recvfrom(sockfd, buffer2, sizeof(buffer2),0, (struct sockaddr*)&servaddr,&len);
                                    if(n<0){
                                            fprintf(stdout, "ERROR: DATA, timeout error %d \n", n);
                                            timeoutCounter ++;
                                    }
                            }
                            if(timeoutCounter >= 2){
                                    ERR_packet_send(sockfd, &servaddr, &cliaddr,len,0);
                                    fprintf(stdout,"ERROR: timeout counter reached, ending transmition\n");
                                    close(sockfd);
                                    exit(1);
                            }

                            // Reset timeout counter
                            timeoutCounter = 0;

                            responceBlockID = ACK_packet_read(buffer2);
                            if( responceBlockID < 0){
                                    ERR_packet_send(sockfd, &servaddr, &cliaddr,len,0);
                                    fprintf(stdout,"ERROR: wrong opcode, ending transmition\n ");
                                    close(sockfd);
                                    exit(1);
                            }

                            if(blockID > responceBlockID){
                                    while(n<=0 && timeoutCounter < 3){
                                            if(sendto(sockfd, dataPacket, sizeOfPacket, 0,(struct sockaddr*)&servaddr, len) < 0){
                                                    fprintf(stdout, "ERROR: DATA, failed to send, errno %d \n", errno);
                                            }

                                            n = recvfrom(sockfd, buffer2, sizeof(buffer2),0, (struct sockaddr*)&servaddr,&len);
                                            if(n<0){
                                                    fprintf(stdout, "ERROR: DATA, timeout error %d \n ", n);
                                                    timeoutCounter ++;
                                            }
                                    } 
                                    if(timeoutCounter >= 2){
                                            ERR_packet_send(sockfd, &cliaddr, &servaddr,len,0);
                                            fprintf(stdout,"ERROR: timeout counter reached, ending transmition\n");
                                            close(sockfd);
                                            exit(1);
                                    }   
                            }
                            else if (blockID == responceBlockID){
                                    ACK_packet_write(inet_ntoa(servaddr.sin_addr),ntohs(servaddr.sin_port),responceBlockID);
                                    free(data);
                                    free(dataPacket);
                                    blockID++;
                            }
                            else {
                                    ERR_packet_send(sockfd, &cliaddr, &servaddr,len,5);
                                    close(sockfd);
                                    exit(1);
                            }
                            n = 0;
                    }
                    
                    fclose(readFromFile);
                    close(sockfd);
                    break;

    ///     END OF START OF DOWNLOAD/UPLOAD LOGIC
    ///
    ////////////////////////////////////
    ///
    ///     START OF CLOSING
    }


    // Close the descriptor on succesful end
    close(sockfd);
}
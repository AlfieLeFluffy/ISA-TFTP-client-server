// udp client driver program
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

#define MAXLINE 1000

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

int handle_options_send(char* _filepath, int* _blockSize, int* _timeout, int* _tsize){
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

        // Set blocksize for better file transfer dependent on size
        if(size>65500){
                *_blockSize=65500;
        }
        else{
                *_blockSize = 512 * (size%512);
        }

        // Check timeout
        if(0>*_timeout>256)*_timeout=1;

        // Set tsize to filesize
        *_tsize = size;
                        
        return 0;
}

int handle_options_recieve(int _blockSize, int _timeout, int _tsize){
        // Check if blocksize is not outside allowed parameters
        if(512>_blockSize>65500)return 8;

        // Check timeout
        if(0>_timeout>256) return 1;

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
    int index;
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
                fileTargetPath = parseInputFilePath(optarg);
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
        fprintf(stdout, "File path is in an incorrect format\n");
        exit(1);
    }
    else if (optind < argc){
        filePathUpload = parseOutputFilePath(argv[optind]);
    }                       

    if (filePathDownload != NULL && filePathUpload != NULL){
        fprintf(stdout, "Can't both download and upload in the same command\n");
        exit(1);
    }

    if (ip == NULL){
        fprintf(stdout, "ERROR: -h is a required argument (IP of server)\n");
        exit(1);
    }

    // Checks if ip address is valid and tries to resolve for hostname if not
    if(!check_ip_valid(ip)){
        ip = resolve_hostname(ip,ip);
        if(ip == "\0")exit(1);
    }

    ///     END OF PARAM PARSE AND CHECK
    ///
    ////////////////////////////////////
    ///
    ///     START OF PROGRAM LOGIC


    // Definition of variables used
    char buffer[512];
    int sockfd, n, sizeOfPacket, opcode, blockID, errorCode;
    unsigned int blockSize,timeout, tsize;
    char* requestPacket;
    char* filePath;
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
    timeout_struct.tv_sec = 3;
    timeout_struct.tv_usec = 0;
    if (setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout_struct,sizeof timeout_struct) < 0) fprintf(stdout,"ERROR : setsocketopt failed, timeout \n");
    
    
    // Connect to server
    while(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        printf("ERROR : connect Failed \n");
        error_exit_FD(1, sockfd);
    }

    // Create packet dependent on which opetation is required 
    if (filePathDownload != NULL){
        opcode = 1;
        requestPacket = RRQ_WRQ_packet_create(&sizeOfPacket,1,filePathDownload,"netascii",65536,1,0);
        filePath = filePathDownload;
    }
    else{
        opcode = 2;
        requestPacket = RRQ_WRQ_packet_create(&sizeOfPacket,2,fileTargetPath,"netascii",256,1,0);
        filePath = filePathUpload;
    }
    sendto(sockfd, requestPacket, sizeOfPacket, 0, (struct sockaddr*)NULL, sizeof(servaddr));
    free(requestPacket);
    close(sockfd);

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
    

    // Receive responce from server
    int len = sizeof(servaddr);
    n = recvfrom(sockfd, buffer, sizeof(buffer),0, (struct sockaddr*)&servaddr,&len); 


    char errorMessage[sizeof(buffer)];
    bzero(&errorMessage, sizeof(errorMessage));
    
    switch (buffer[1]){
        case 4:
            blockID = ACK_packet_read(buffer);
            ACK_message_write(inet_ntoa(servaddr.sin_addr),servaddr.sin_port, blockID);
            break;
        case 5:
            errorCode = ERR_packet_read(buffer, errorMessage);
            ERR_message_write(inet_ntoa(servaddr.sin_addr),servaddr.sin_port, cliaddr.sin_port,errorCode,errorMessage);
            error_exit_FD(errorCode, sockfd);
            break;
        case 6:
            errorCode = OACK_packet_read(buffer,n,&blockSize,&timeout,&tsize);
            if(errorCode){
                fprintf(stdout, "ERROR: handling OACK responce: %d\n", errorCode);
                close(sockfd);
                exit(1);
            }
            OACK_message_write(inet_ntoa(servaddr.sin_addr),servaddr.sin_port,blockSize,timeout,tsize);
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


    // Close the descriptor on succesful end
    close(sockfd);
}
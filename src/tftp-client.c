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

#include "../include/common.h"
#include "../include/parser.h"
#include "../include/request_pack.h"
#include "../include/error_pack.h"

#define MAXLINE 1000
  

int main(int argc, char *argv[]) 
{
    char *ip = NULL;
    int port = 69;
    char *filePathDownload = NULL;
    char *filePathUpload = NULL;
    char *targetPath = NULL;
    int index;
    int c;

    opterr = 0;


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
                targetPath = optarg;
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

    if(optind+1 < argc){
        fprintf(stdout, "File path is in an incorrect format\n");
        exit(1);
    }
    else if (optind < argc){
        filePathUpload = parseFilePath(argv[optind]);
    }

    if (filePathDownload != NULL && filePathUpload != NULL){
        fprintf(stdout, "Can't both download and upload in the same command\n");
        exit(1);
    }

    if (ip == NULL){
        fprintf(stdout, "ERROR: -h is a required argument (IP of server)\n");
        exit(1);
    }

    // definition of variables used
    char buffer[100];
    char *message = "\0";
    int sockfd, n;
    struct sockaddr_in servaddr, clientaddr;
        
    // clear servaddr
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_addr.s_addr = inet_addr(ip);
    servaddr.sin_port = htons(port);
    servaddr.sin_family = AF_INET;

    bzero(&clientaddr, sizeof(clientaddr));
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);        
    clientaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    clientaddr.sin_port = htons(port-1);
    clientaddr.sin_family = AF_INET; 

    // bind client address to socket descriptor
    if(bind(sockfd, (struct sockaddr*)&clientaddr, sizeof(clientaddr))){
            fprintf(stdout, "ERROR : creating socket, could not bind\n");
            exit(1);
    }
        
    // connect to server
    while(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        printf("ERROR : connect Failed \n");
        error_exit_FD(1, sockfd);
    }

    int sizeOfPacket;
    char* requestPacket;

    if (filePathDownload != NULL){
        requestPacket = RRQ_WRQ_packet_create(&sizeOfPacket,1,filePathDownload,"netascii");
    }
    else{
        requestPacket = RRQ_WRQ_packet_create(&sizeOfPacket,2,filePathUpload,"netascii");
    }

    // request to send datagram
    // no need to specify server address in sendto
    // connect stores the peers IP and port
    sendto(sockfd, requestPacket, sizeOfPacket, 0, (struct sockaddr*)NULL, sizeof(servaddr));

    fprintf(stdout, "Request sent to %s:%d\n", inet_ntoa(servaddr.sin_addr), servaddr.sin_port);
        
    // waiting for response

    //receive message from server
    int len = sizeof(servaddr);
    n = recvfrom(sockfd, buffer, sizeof(buffer),0, (struct sockaddr*)&servaddr,&len); 

    switch (buffer[1]){
        case 4:
            break;
        case 5:
            char errorMessage[n];
            bzero(&errorMessage, sizeof(errorMessage));
            int errorCode = ERR_packet_read(buffer, errorMessage);
            ERR_message_write(inet_ntoa(servaddr.sin_addr),servaddr.sin_port, clientaddr.sin_port,errorCode,errorMessage);
            error_exit_FDFP(errorCode, sockfd, requestPacket);
            break;
    }


    // close the descriptor
    free(requestPacket);
    close(sockfd);
}
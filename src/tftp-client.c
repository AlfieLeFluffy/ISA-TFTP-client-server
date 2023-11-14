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

#include "../include/common.h"
#include "../include/parser.h"

#include "../include/request_pack.h"
#include "../include/error_pack.h"
#include "../include/ack_pack.h"
#include "../include/data_pack.h"

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

    /*
        Checks if ip address is valid and tries to resolve for hostname if not
    */
    if(!check_ip_valid(ip)){
        ip = resolve_hostname(ip,ip);
        if(ip == "\0")exit(1);
    }

    ///     END OF PARAM PARSE AND CHECK
    ///
    ////////////////////////////////////
    ///
    ///     START OF PROGRAM LOGIC

    ///     SETUP OF SOCKETS AND TARGER SOCKADDR
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

    /// CREATING AND SENDING OF RRQ/WRQ PACKET
    int sizeOfPacket, opcode;
    char* requestPacket;
    char* filePath;

    if (filePathDownload != NULL){
        requestPacket = RRQ_WRQ_packet_create(&sizeOfPacket,1,filePathDownload,"netascii");
        opcode = 1;
        filePath = filePathDownload;
    }
    else{
        requestPacket = RRQ_WRQ_packet_create(&sizeOfPacket,2,fileTargetPath,"netascii");
        opcode = 2;
        filePath = filePathUpload;
    }

    // request to send datagram
    // no need to specify server address in sendto
    // connect stores the peers IP and port
    sendto(sockfd, requestPacket, sizeOfPacket, 0, (struct sockaddr*)NULL, sizeof(servaddr));
    RRQ_WRQ_request_write(opcode, &clientaddr, filePath, "netascii");
    free(requestPacket);
        
    /// WAITING FOR SERVER RESPONSE AND PARSING IT
    //receive message from server
    int len = sizeof(servaddr);
    n = recvfrom(sockfd, buffer, sizeof(buffer),0, (struct sockaddr*)&servaddr,&len); 


    char errorMessage[sizeof(buffer)];
    bzero(&errorMessage, sizeof(errorMessage));
    
    switch (buffer[1]){
        case 4:
            int blockID = ACK_packet_read(buffer);
            ACK_message_write(inet_ntoa(servaddr.sin_addr),servaddr.sin_port, blockID);
            break;
        case 5:
            int errorCode = ERR_packet_read(buffer, errorMessage);
            ERR_message_write(inet_ntoa(servaddr.sin_addr),servaddr.sin_port, clientaddr.sin_port,errorCode,errorMessage);
            error_exit_FD(errorCode, sockfd);
            break;
        case 6:
            break;
        default:
            fprintf(stdout, "ERROR: internal error (wrong answer OPCODE)\n");
            error_exit_FD(1,sockfd);
            break;
    }


    // close the descriptor on succesful end
    close(sockfd);
}
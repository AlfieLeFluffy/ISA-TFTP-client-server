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
#include "../include/packet-stuct.h"

#define MAXLINE 1000
  

int main(int argc, char *argv[]) 
{
    char *ip = NULL;
    int port = 69;
    char *filePathOutput = NULL;
    char *filePathInput = NULL;
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
                filePathOutput = optarg;
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
    else if (filePathOutput != NULL && optind < argc){
        fprintf(stdout, "Can't both download and upload in the same command\n");
        exit(1);
    }
    else if (filePathOutput != NULL){
        filePathInput = parseFilePath(argv[optind]);
    }


    char buffer[100];
    char *message = "Hello Server\0";
    int sockfd, n;
    struct sockaddr_in servaddr;
        
    // clear servaddr
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_addr.s_addr = inet_addr(ip);
    servaddr.sin_port = htons(port);
    servaddr.sin_family = AF_INET;
        
    // create datagram socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        
    // connect to server
    if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        printf("\n Error : Connect Failed \n");
        exit(0);
    }

    struct sockaddr_in *sin = (struct sockaddr_in *)&servaddr;
    fprintf(stdout, "Request sent to %s:%d\n", inet_ntoa(sin->sin_addr), sin->sin_port);

    // request to send datagram
    // no need to specify server address in sendto
    // connect stores the peers IP and port
    sendto(sockfd, message, strlen(message), 0, (struct sockaddr*)NULL, sizeof(servaddr));
        
    // waiting for response
    recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)NULL, NULL);
    puts(buffer);

    // close the descriptor
    close(sockfd);
}
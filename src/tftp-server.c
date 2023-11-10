// server program for udp connection
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

#include "../include/common.h"
#include "../include/parser.h"
#include "../include/packet-stuct.h"


#define MAXLINE 1000
  
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
        struct sockaddr_in servaddr, cliaddr;
        bzero(&servaddr, sizeof(servaddr));

        // Create a UDP Socket
        listenfd = socket(AF_INET, SOCK_DGRAM, 0);        
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        servaddr.sin_port = htons(port);
        servaddr.sin_family = AF_INET; 

        // bind server address to socket descriptor
        if(bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr))){
                fprintf(stdout, "Error creating socket, could not bind\n");
                exit(1);
        }

        while(1){
                //receive the datagram
                len = sizeof(cliaddr);
                int n = recvfrom(listenfd, buffer, sizeof(buffer),0, (struct sockaddr*)&cliaddr,&len); //receive message from server
                
                while(!fork()){

                        struct sockaddr_in *sin = (struct sockaddr_in *)&cliaddr;

                        char *filename;
                        char *mode;
                        int opcode = RRQ_WRQ_packet_read(buffer, filename, mode);

                        for (int i = 0; i< n; i++){
                                printf("%c ", buffer[i]);
                        }

                        fprintf(stdout, "Incoming request from %s:%d of OPCODE: %d on a file: %s in mode %s \n", inet_ntoa(sin->sin_addr), sin->sin_port, opcode, filename, mode);


                                
                        // send the response
                        char *message = "Hello Client\0";
                        sendto(listenfd, message, strlen(message), 0,
                                (struct sockaddr*)&cliaddr, sizeof(cliaddr));

                        return 0;
                }
        }
        return 0;
}
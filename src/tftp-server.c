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

#include "../include/common.h"
#include "../include/packet-stuct.h"

#define MAXLINE 1000
  
int main(int argc, char *argv[]) 
{
        int port = 69;
        int index;
        int c;
        opterr = 0;


        while ((c = getopt (argc, argv, "p:")) != -1)
                switch (c){
                        case 'p':
                                port = atoi(optarg);
                                break;
                        case '?':
                                if (optopt == 'c')
                                        fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                                else if (isprint (optopt))
                                        fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                                else
                                        fprintf (stderr,
                                                "Unknown option character `\\x%x'.\n",
                                                optopt);
                                        return 1;
                        default:
                                abort ();       
        }

        for (index = optind; index < argc; index++)
                printf ("Non-option argument %s\n", argv[index]);

        char buffer[100];

        for (int i = 0; i<100;i++){
                buffer[i] = '\0';
        }       

        char *message = "Hello Client\0";
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
                int n = recvfrom(listenfd, buffer, sizeof(buffer),
                        0, (struct sockaddr*)&cliaddr,&len); //receive message from server
                
                while(!fork()){
                        buffer[n] = '\0';
                        puts(buffer);

                        struct sockaddr_in *sin = (struct sockaddr_in *)&cliaddr;
                        fprintf(stdout, "Incoming request from %s:%d\n", inet_ntoa(sin->sin_addr), sin->sin_port);

                        FILE* file = fopen("./testOutput.txt", "wb");
                        for (int i = 0; i<n; i++){
                                fputc(buffer[i], file);
                        }
                        fclose(file);
                                
                        // send the response
                        sendto(listenfd, message, MAXLINE, 0,
                                (struct sockaddr*)&cliaddr, sizeof(cliaddr));

                        return 0;
                }
        }
        return 0;
}
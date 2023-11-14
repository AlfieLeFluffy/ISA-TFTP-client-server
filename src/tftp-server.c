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

#include "../include/error_code_msg.h"

#include "../include/request_pack.h"
#include "../include/error_pack.h"



int handle_TFTP_request(int _opcode, char* _filePath, char* _mode){
        if(_opcode != 1 && _opcode != 2){
                fprintf(stdout, "ERROR: opcode for request does not match possible opcodes\n");
                return 4;
        }

        switch(_opcode){
                case 1:
                        if (access(_filePath, F_OK) != 0) return 1;
                        break;
                case 2:
                        break;
                default:
                        fprintf(stdout, "ERROR: Internal error\n");
                        return 1;
                        break;
        }
}

char* create_file_path(char* _filename, char* _folderPath){
        char* filePath = (char*)malloc(sizeof(_filename)+sizeof(_folderPath)+2);
        
        strcat(filePath, _folderPath);
        if(_filename[0] != '/') strcat(filePath, "/");
        strcat(filePath, _filename);
        strcat(filePath, "\0");
        return filePath;
}


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

        
        int listenfd, len, requestNum;
        struct sockaddr_in servaddr, cliaddr;

        bzero(&servaddr, sizeof(servaddr));
        requestNum = 0;

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
                requestNum++;
                
                while(!fork()){

                        struct sockaddr_in *sin = (struct sockaddr_in *)&cliaddr;

                        //      write out the request buffer
                        /*for (int i = 0; i< n; i++){
                                printf("%d ", buffer[i]);
                        }
                        fprintf(stdout, "\n");*/

                        char filename[n];
                        bzero(&filename, sizeof(filename));
                        char mode[50];
                        bzero(&mode, sizeof(mode));
                        int opcode = RRQ_WRQ_packet_read(buffer, filename, mode);
                        char* filePath = create_file_path(filename, folderPath);

                        RRQ_WRQ_request_write(opcode, sin, filePath, mode);

                        
                        int errorCode;
                        if(errorCode = handle_TFTP_request(opcode, filePath, mode)){
                                int sizeOfPacket;
                                char* errorPacket = ERR_packet_create(&sizeOfPacket, errorCode, errorMessage[errorCode]);
                                sendto(listenfd, errorPacket, sizeOfPacket, 0,(struct sockaddr*)&cliaddr, sizeof(cliaddr));
                                fprintf(stdout, "%d. ERROR: handling a TFTP request, error code:%d\n", requestNum, errorCode);
                                return -1;
                        }

                        


                                
                        // send the response
                        char *message = "Hello Client\0";
                        sendto(listenfd, message, strlen(message), 0,
                                (struct sockaddr*)&cliaddr, sizeof(cliaddr));

                        return 0;
                }
        }
        return 0;
}
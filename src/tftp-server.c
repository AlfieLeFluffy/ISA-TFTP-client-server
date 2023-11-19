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
                                printf("ERROR: COuld not open file\n"); 
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

                        break;
                case 2:
                        // Check if blocksize is not outside allowed parameters
                        if(512>*_blockSize>65500)return 8;

                        // Check timeout
                        if(0>*_timeout>256)*_timeout=1;

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
        if(_filename[0] != '/') strcat(filePath, "/");
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
                        int opcode, sizeOfPacket, blockSize, timeout, tsize, errorCode;
                        char* filePath;
                        char filename[n], mode[50];
                        struct timeval timeout_struct; 

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
                        RRQ_WRQ_request_write(opcode, clientaddr_in, filePath, mode);
                        
                        if(errorCode = handle_options(opcode, folderPath, filePath, &blockSize, &timeout, &tsize)){
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

                        

                        // Switch and handle the file transfer 
                        switch(opcode){
                                case 1:
                                        send_file(listenfd, &servaddr, &cliaddr, sizeof(cliaddr), filePath, mode, blockSize);
                                        break;
                                case 2:
                                        recieve_file(listenfd, &servaddr, &cliaddr, sizeof(cliaddr), filePath, mode, blockSize);
                                        break;
                        }

                        return 0;

                }
        }
        return 0;
}
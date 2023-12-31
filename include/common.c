///////////////////////////////////////////////////////////////////////////////////////////
///                                                                                     ///
///     TFTP server/client include                                                      ///
///                                                                                     ///
///     vytvoril: Tomas Vlach                                                           ///
///     login: xvlach24                                                                 ///
///                                                                                     ///
///////////////////////////////////////////////////////////////////////////////////////////



#include <stdio.h>
#include <stdlib.h>

void error_exit_FDFP(int error_type, int sockfd, char* requestPacket){
    free(requestPacket);
    close(sockfd);
    exit(error_type);
}

void error_exit_FD(int error_type,int sockfd){
    close(sockfd);
    exit(error_type);
}

void append_char(char* s, char c) {
        int len = strlen(s);
        s[len] = c;
}




#include <stdio.h>
#include <stdlib.h>

void error_exit(int error_type){
    fprintf(stderr, "Error number %d\n", error_type);
    exit(error_type);
}


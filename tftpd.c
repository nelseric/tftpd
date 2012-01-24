#include "tftpd.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char *argv[]){
    
    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);


    return EXIT_SUCCESS;
}

